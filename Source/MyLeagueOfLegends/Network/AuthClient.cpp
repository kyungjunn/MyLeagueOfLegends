#include "Network/AuthClient.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "TimerManager.h"

//void UAuthClient::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//	ConnectionSocket = nullptr;
//}
//
//void UAuthClient::Deinitialize()
//{
//	CloseConnection();
//	Super::Deinitialize();
//}

// 편의를 위해 매번 서버 정보를 칠 필요 없이 디폴트 IP/포트로 연결해주는 래퍼
void UAuthClient::Login(const FString& UserId, const FString& Password)
{
	// 본인의 C++ 백엔드 서버 IP와 ListenPort(9000)를 입력하세요.
	//RequestLogin(TEXT("127.0.0.1"), 9000, UserId, Password);
	RequestLogin(TEXT("192.168.0.97"), 9000, UserId, Password);
}

void UAuthClient::Signup(const FString& UserId, const FString& Password, const FString& Nickname)
{
	//RequestSignup(TEXT("127.0.0.1"), 9000, UserId, Password, Nickname);
	RequestSignup(TEXT("192.168.0.97"), 9000, UserId, Password, Nickname);
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
}

void UAuthClient::RequestLogin(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password)
{
	if (!ConnectToServer(ServerIP, Port))
	{
		OnLoginResult.Broadcast(false, TEXT(""), TEXT("auth server dont connect"));
		return;
	}

	// 서버 데이터 구조와 완벽 매칭되도록 패킷 정보 구성 ("type", "id", "password")
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("type"), TEXT("LOGIN_REQUEST"));
	JsonObject->SetStringField(TEXT("id"), UserId);
	JsonObject->SetStringField(TEXT("password"), Password);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	SendJsonString(OutputString);
}

void UAuthClient::RequestSignup(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password, const FString& Nickname)
{
	if (!ConnectToServer(ServerIP, Port))
	{
		OnSignupResult.Broadcast(false, TEXT("auth server dont connect"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("type"), TEXT("REGISTER_REQUEST"));
	JsonObject->SetStringField(TEXT("id"), UserId);
	JsonObject->SetStringField(TEXT("password"), Password);
	JsonObject->SetStringField(TEXT("nickname"), Nickname);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	SendJsonString(OutputString);
}

bool UAuthClient::SendJsonString(const FString& JsonString)
{
	if (!ConnectionSocket) return false;

	// JSON 문자열을 UTF-8 바이트 배열 변환.
	FTCHARToUTF8 Converter(*JsonString);
	uint32 BodyLength = static_cast<uint32>(Converter.Length());

	if (BodyLength == 0) return false;

	// 2. 서버의 맞춰 길이를 빅엔디안 직렬화
	uint8 Header[4];
	Header[0] = static_cast<uint8>((BodyLength >> 24) & 0xFF);
	Header[1] = static_cast<uint8>((BodyLength >> 16) & 0xFF);
	Header[2] = static_cast<uint8>((BodyLength >> 8) & 0xFF);
	Header[3] = static_cast<uint8>(BodyLength & 0xFF);

	// 총 전송 버퍼 조립 (헤더 4바이트 + 본문 데이터)
	TArray<uint8> PacketBuffer;
	PacketBuffer.Append(Header, 4);
	PacketBuffer.Append(reinterpret_cast<const uint8*>(Converter.Get()), BodyLength);

	// 서버로 한번에 스트림 전송
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
		// 먼저 헤더 4바이트만 훔쳐봐서 데이터 전체 크기 읽기. (Peek 모드)
		TArray<uint8> HeaderBuffer;
		HeaderBuffer.SetNum(4);
		int32 BytesRead = 0;

		if (ConnectionSocket->Recv(HeaderBuffer.GetData(), 4, BytesRead, ESocketReceiveFlags::Peek))
		{
			// 빅엔디안 헤더를 정수로 복원
			uint32 BodyLength = (static_cast<uint32>(HeaderBuffer[0]) << 24) |
				(static_cast<uint32>(HeaderBuffer[1]) << 16) |
				(static_cast<uint32>(HeaderBuffer[2]) << 8) |
				static_cast<uint32>(HeaderBuffer[3]);

			// 전체 패킷(헤더 4바이트 + 데이터 본문 크기)이 버퍼에 다 쌓였는지 확인.
			if (PendingDataSize >= (4 + BodyLength))
			{
				// 완전히 다 왔다면 Peek가 아닌 실제 Recv으로 버퍼 비우기.
				TArray<uint8> FullPacketBuffer;
				FullPacketBuffer.SetNum(4 + BodyLength);

				if (ConnectionSocket->Recv(FullPacketBuffer.GetData(), FullPacketBuffer.Num(), BytesRead))
				{
					// 헤더 4바이트 뒤에 있는 순수 JSON 데이터만 추출
					TArray<uint8> JsonBytes;
					JsonBytes.Append(FullPacketBuffer.GetData() + 4, BodyLength);
					JsonBytes.Add(0); 

					FString ReceivedString = FString(UTF8_TO_TCHAR((const char*)JsonBytes.GetData()));

					// 최종 문자열 파싱 처리
					ProcessReceivedPacket(ReceivedString);
				}
			}
		}
	}
}

void UAuthClient::ProcessReceivedPacket(const FString& JsonData)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		FString Type = JsonObject->GetStringField(TEXT("type"));

		if (Type == TEXT("LOGIN_RESPONSE"))
		{
			bool bSuccess = JsonObject->GetBoolField(TEXT("success"));
			FString Nickname = JsonObject->GetStringField(TEXT("nickname"));
			FString ErrorMessage = JsonObject->GetStringField(TEXT("errorMessage"));

			// 위젯단으로 결과 전파
			OnLoginResult.Broadcast(bSuccess, Nickname, bSuccess ? TEXT("Login Success!") : ErrorMessage);

			// 로그인 이후 게임 루프로 넘어가므로 소켓 세션을 정리해 줍니다.
			CloseConnection();
		}
		else if (Type == TEXT("REGISTER_RESPONSE"))
		{
			bool bSuccess = JsonObject->GetBoolField(TEXT("success"));
			FString ErrorMessage = JsonObject->GetStringField(TEXT("errorMessage"));

			OnSignupResult.Broadcast(bSuccess, bSuccess ? TEXT("Signup Completed") : ErrorMessage);
			CloseConnection();
		}
	}
}