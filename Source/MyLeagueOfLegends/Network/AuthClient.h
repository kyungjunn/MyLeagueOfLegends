// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sockets.h"
#include "Network/PacketOpcodes.h"
#include "AuthClient.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoginResult, bool, bSuccess, const FString&, Nickname, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSignupResult, bool, bSuccess, const FString&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatJoinRoomResult, bool, bSuccess, const FString&, RoomName, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatLeaveRoomResult, bool, bSuccess, const FString&, RoomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnChatMessageReceived, const FString&, RoomName, const FString&, Sender, const FString&, Message, const FString&, Timestamp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatSystemMessage, const FString&, RoomName, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthError, const FString&, ErrorMessage);

UCLASS(BlueprintType)
class MYLEAGUEOFLEGENDS_API UAuthClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 서브시스템 초기화/해제 라이프사이클. 로그인 이후에도 소켓을 계속 들고 있을 수 있으므로,
	// 게임 인스턴스 종료 시 Deinitialize에서 반드시 소켓을 정리해야 함.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnLoginResult OnLoginResult;

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnSignupResult OnSignupResult;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatJoinRoomResult OnChatJoinRoomResult;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatLeaveRoomResult OnChatLeaveRoomResult;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceived OnChatMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatSystemMessage OnChatSystemMessage;

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnAuthError OnAuthError;

	// 기존 LoginWidget에서 매번 서버 정보를 칠 필요 없이 디폴트 IP/포트로 연결해주는 래퍼
	void Login(const FString& UserId, const FString& Password);
	void Signup(const FString& UserId, const FString& Password, const FString& Nickname);

	// 로그인 요청
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void RequestLogin(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password);

	// 회원가입 요청
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void RequestSignup(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password, const FString& Nickname);

	// 로그인 성공 후 같은 소켓으로 방 채팅에 참가/퇴장/메시지 전송.
	// 로그인되지 않은 상태에서 호출하면 OnAuthError만 브로드캐스트하고 아무것도 보내지 않음
	// (서버도 어차피 인증 상태를 재검증하므로, 이건 방어적 체크일 뿐).
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void JoinChatRoom(const FString& RoomName);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void LeaveChatRoom(const FString& RoomName);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FString& RoomName, const FString& Message);

private:
	// 서버와의 TCP 연결 소켓 정보
	FSocket* ConnectionSocket;

	// 비동기로 오는 서버 응답 패킷을 수신하기 위한 폴링/타이머 풀링용 함수
	FTimerHandle SocketPollTimerHandle;
	void PollSocketData();

	// 연결과 송수신 관련 함수
	bool ConnectToServer(const FString& ServerIP, int32 Port);
	void CloseConnection();
	bool SendPacket(EPacketOpcode Opcode, const TSharedRef<FJsonObject>& Body);
	void ProcessReceivedPacket(EPacketOpcode Opcode, const FString& JsonData);

	bool bIsAuthenticated = false;
	FString CurrentRoomName;
};
