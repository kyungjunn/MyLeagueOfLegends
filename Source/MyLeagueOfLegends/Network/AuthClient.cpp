#include "Network/AuthClient.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "TimerManager.h"

void UAuthClient::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ConnectionSocket = nullptr;
}

void UAuthClient::Deinitialize()
{
	// 로그인 이후에도 소켓을 계속 열어두는 구조라, 게임 인스턴스 종료 시 여기서 정리해주지 않으면
	// TCP 연결이 그대로 남는다.
	CloseConnection();
	Super::Deinitialize();
}

// 편의를 위해 매번 서버 정보를 칠 필요 없이 디폴트 IP/포트로 연결해주는 래퍼
void UAuthClient::Login(const FString& UserId, const FString& Password)
{
	// 본인의 C++ 백엔드 서버 IP와 ListenPort(9000)를 입력하세요.
	RequestLogin(TEXT("127.0.0.1"), 9000, UserId, Password);
	//RequestLogin(TEXT("192.168.0.97"), 9000, UserId, Password);
}

void UAuthClient::Signup(const FString& UserId, const FString& Password, const FString& Nickname)
{
	RequestSignup(TEXT("127.0.0.1"), 9000, UserId, Password, Nickname);
	//RequestSignup(TEXT("192.168.0.97"), 9000, UserId, Password, Nickname);
}

bool UAuthClient::ConnectToServer(const FString& ServerIP, int32 Port)
{
	if (ConnectionSocket) return true; // 이미 연결됨

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem) return false;

	FIPv4Address Address;
	if (!FIPv4Address::Parse(ServerIP, Address)) return false;

	TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
	Addr->SetIp(Address.Value);
	Addr->SetPort(Port);

	ConnectionSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("AuthServerSocket"), false);
	if (!ConnectionSocket) return false;

	// 블로킹 모드로 연결 시도
	if (!ConnectionSocket->Connect(*Addr))
	{
		CloseConnection();
		return false;
	}

	// 서버 데이터 수신을 감지할 타이머 작동 (0.02초 간격 풀링)
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().SetTimer(SocketPollTimerHandle, this, &UAuthClient::PollSocketData, 0.02f, true);
	}

	return true;
}

void UAuthClient::CloseConnection()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(SocketPollTimerHandle);
	}

	if (ConnectionSocket)
	{
		ConnectionSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		ConnectionSocket = nullptr;
	}

	bIsAuthenticated = false;
	CurrentRoomName.Empty();
}

void UAuthClient::RequestLogin(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password)
{
	if (!ConnectToServer(ServerIP, Port))
	{
		OnLoginResult.Broadcast(false, TEXT(""), TEXT("auth server dont connect"));
		return;
	}

	// opcode(LOGIN_REQUEST)가 이제 메시지 종류를 나타내므로, JSON 본문에는 실제 필드만 담음
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("id"), UserId);
	JsonObject->SetStringField(TEXT("password"), Password);

	SendPacket(EPacketOpcode::LoginRequest, JsonObject.ToSharedRef());
}

void UAuthClient::RequestSignup(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password, const FString& Nickname)
{
	if (!ConnectToServer(ServerIP, Port))
	{
		OnSignupResult.Broadcast(false, TEXT("auth server dont connect"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("id"), UserId);
	JsonObject->SetStringField(TEXT("password"), Password);
	JsonObject->SetStringField(TEXT("nickname"), Nickname);

	SendPacket(EPacketOpcode::RegisterRequest, JsonObject.ToSharedRef());
}

void UAuthClient::JoinChatRoom(const FString& RoomName)
{
	if (!bIsAuthenticated || !ConnectionSocket)
	{
		OnAuthError.Broadcast(TEXT("not authenticated"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("roomName"), RoomName);

	SendPacket(EPacketOpcode::ChatJoinRoom, JsonObject.ToSharedRef());
}

void UAuthClient::LeaveChatRoom(const FString& RoomName)
{
	if (!bIsAuthenticated || !ConnectionSocket)
	{
		OnAuthError.Broadcast(TEXT("not authenticated"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("roomName"), RoomName);

	SendPacket(EPacketOpcode::ChatLeaveRoom, JsonObject.ToSharedRef());
}

void UAuthClient::SendChatMessage(const FString& RoomName, const FString& Message)
{
	if (!bIsAuthenticated || !ConnectionSocket)
	{
		OnAuthError.Broadcast(TEXT("not authenticated"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("roomName"), RoomName);
	JsonObject->SetStringField(TEXT("message"), Message);

	SendPacket(EPacketOpcode::ChatMessage, JsonObject.ToSharedRef());
}

bool UAuthClient::SendPacket(EPacketOpcode Opcode, const TSharedRef<FJsonObject>& Body)
{
	if (!ConnectionSocket) return false;

	// 1. JSON 본문을 UTF-8 바이트 배열로 직렬화합니다.
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Body, Writer);

	FTCHARToUTF8 Converter(*OutputString);
	const uint32 BodyLength = static_cast<uint32>(Converter.Length());

	// 2. [4B 길이][2B opcode][JSON body] 프레임을 조립합니다.
	//    길이는 opcode(2B) + body 길이의 합 (길이 필드 자신은 포함하지 않음) -- 서버의
	//    Protocol::PacketFramer와 동일한 규약.
	const uint16 OpcodeValue = static_cast<uint16>(Opcode);
	const uint32 TotalLength = 2 + BodyLength;

	TArray<uint8> PacketBuffer;
	PacketBuffer.Reserve(4 + TotalLength);

	PacketBuffer.Add(static_cast<uint8>((TotalLength >> 24) & 0xFF));
	PacketBuffer.Add(static_cast<uint8>((TotalLength >> 16) & 0xFF));
	PacketBuffer.Add(static_cast<uint8>((TotalLength >> 8) & 0xFF));
	PacketBuffer.Add(static_cast<uint8>(TotalLength & 0xFF));

	PacketBuffer.Add(static_cast<uint8>((OpcodeValue >> 8) & 0xFF));
	PacketBuffer.Add(static_cast<uint8>(OpcodeValue & 0xFF));

	if (BodyLength > 0)
	{
		PacketBuffer.Append(reinterpret_cast<const uint8*>(Converter.Get()), BodyLength);
	}

	// 3. 서버로 한번에 스트림 전송
	int32 BytesSent = 0;
	return ConnectionSocket->Send(PacketBuffer.GetData(), PacketBuffer.Num(), BytesSent);
}

void UAuthClient::PollSocketData()
{
	if (!ConnectionSocket) return;

	uint32 PendingDataSize = 0;
	// 읽을 데이터가 존재하고, 최소 헤더 크기(4바이트) 이상 쌓였을 때만 처리 시작
	if (ConnectionSocket->HasPendingData(PendingDataSize) && PendingDataSize >= 4)
	{
		// 1. 먼저 헤더 4바이트만 훔쳐봐서 (opcode + body) 전체 크기를 읽어냅니다. (Peek 모드)
		TArray<uint8> HeaderBuffer;
		HeaderBuffer.SetNum(4);
		int32 BytesRead = 0;

		if (ConnectionSocket->Recv(HeaderBuffer.GetData(), 4, BytesRead, ESocketReceiveFlags::Peek))
		{
			// 빅엔디안 헤더를 정수로 복원
			const uint32 TotalLength = (static_cast<uint32>(HeaderBuffer[0]) << 24) |
				(static_cast<uint32>(HeaderBuffer[1]) << 16) |
				(static_cast<uint32>(HeaderBuffer[2]) << 8) |
				static_cast<uint32>(HeaderBuffer[3]);

			// 2. 전체 패킷(헤더 4바이트 + opcode 2바이트 + JSON 본문)이 버퍼에 다 쌓였는지 확인합니다.
			if (TotalLength >= 2 && PendingDataSize >= (4 + TotalLength))
			{
				// 완전히 다 왔다면 Peek가 아닌 실제 수신(Recv)으로 버퍼를 비워냅니다.
				TArray<uint8> FullPacketBuffer;
				FullPacketBuffer.SetNum(4 + TotalLength);

				if (ConnectionSocket->Recv(FullPacketBuffer.GetData(), FullPacketBuffer.Num(), BytesRead))
				{
					// 헤더 4바이트 뒤의 2바이트가 opcode, 그 뒤가 JSON 본문
					const uint16 OpcodeValue = (static_cast<uint16>(FullPacketBuffer[4]) << 8) |
						static_cast<uint16>(FullPacketBuffer[5]);

					const int32 JsonBodyLength = static_cast<int32>(TotalLength) - 2;
					TArray<uint8> JsonBytes;
					JsonBytes.Append(FullPacketBuffer.GetData() + 6, JsonBodyLength);
					JsonBytes.Add(0); // C-스타일 스트링을 위한 널 종료 문자

					const FString ReceivedString = FString(UTF8_TO_TCHAR((const char*)JsonBytes.GetData()));

					// 최종 문자열 파싱 처리
					ProcessReceivedPacket(static_cast<EPacketOpcode>(OpcodeValue), ReceivedString);
				}
			}
		}
	}
}

void UAuthClient::ProcessReceivedPacket(EPacketOpcode Opcode, const FString& JsonData)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	switch (Opcode)
	{
	case EPacketOpcode::LoginResponse:
	{
		const bool bSuccess = JsonObject->GetBoolField(TEXT("success"));
		const FString Nickname = JsonObject->GetStringField(TEXT("nickname"));
		const FString ErrorMessage = JsonObject->GetStringField(TEXT("errorMessage"));

		if (bSuccess)
		{
			bIsAuthenticated = true;
		}

		// 위젯단으로 결과 전파
		OnLoginResult.Broadcast(bSuccess, Nickname, bSuccess ? TEXT("Login Success!") : ErrorMessage);

		// 로그인 성공 시에는 소켓을 닫지 않음 -- 같은 연결을 이어서 방 채팅에 사용하기 위함.
		break;
	}
	case EPacketOpcode::RegisterResponse:
	{
		const bool bSuccess = JsonObject->GetBoolField(TEXT("success"));
		const FString ErrorMessage = JsonObject->GetStringField(TEXT("errorMessage"));

		OnSignupResult.Broadcast(bSuccess, bSuccess ? TEXT("Signup Completed") : ErrorMessage);

		// 회원가입 시점에는 아직 로그인 상태가 아니라 소켓을 열어둘 이유가 없으므로 기존처럼 정리.
		CloseConnection();
		break;
	}
	case EPacketOpcode::ChatJoinRoomResponse:
	{
		const bool bSuccess = JsonObject->GetBoolField(TEXT("success"));
		const FString RoomName = JsonObject->GetStringField(TEXT("roomName"));
		const FString ErrorMessage = JsonObject->GetStringField(TEXT("errorMessage"));

		if (bSuccess)
		{
			CurrentRoomName = RoomName;
		}

		OnChatJoinRoomResult.Broadcast(bSuccess, RoomName, ErrorMessage);
		break;
	}
	case EPacketOpcode::ChatLeaveRoomResponse:
	{
		const bool bSuccess = JsonObject->GetBoolField(TEXT("success"));
		const FString RoomName = JsonObject->GetStringField(TEXT("roomName"));

		if (bSuccess && CurrentRoomName == RoomName)
		{
			CurrentRoomName.Empty();
		}

		OnChatLeaveRoomResult.Broadcast(bSuccess, RoomName);
		break;
	}
	case EPacketOpcode::ChatMessageBroadcast:
	{
		const FString RoomName = JsonObject->GetStringField(TEXT("roomName"));
		const FString Sender = JsonObject->GetStringField(TEXT("sender"));
		const FString Message = JsonObject->GetStringField(TEXT("message"));
		const FString Timestamp = JsonObject->GetStringField(TEXT("timestamp"));

		OnChatMessageReceived.Broadcast(RoomName, Sender, Message, Timestamp);
		break;
	}
	case EPacketOpcode::ChatSystemMessage:
	{
		const FString RoomName = JsonObject->GetStringField(TEXT("roomName"));
		const FString Message = JsonObject->GetStringField(TEXT("message"));

		OnChatSystemMessage.Broadcast(RoomName, Message);
		break;
	}
	case EPacketOpcode::ErrorResponse:
	{
		const FString ErrorMessage = JsonObject->GetStringField(TEXT("errorMessage"));
		OnAuthError.Broadcast(ErrorMessage);
		break;
	}
	default:
		break;
	}
}
