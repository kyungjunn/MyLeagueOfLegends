// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sockets.h"
#include "AuthClient.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoginResult, bool, bSuccess, const FString&, Nickname, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSignupResult, bool, bSuccess, const FString&, Message);

UCLASS(BlueprintType)
class MYLEAGUEOFLEGENDS_API UAuthClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 서브시스템 초기화/해제 라이프사이클 (필요 시 구현, 여기선 기본 제공 사용)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override { Super::Initialize(Collection); }
	virtual void Deinitialize() override { Super::Deinitialize(); }

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnLoginResult OnLoginResult;

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnSignupResult OnSignupResult;

	// 기존 LoginWidget에서 쓰던 형태에 맞춰 헬퍼 함수 정의
	void Login(const FString& UserId, const FString& Password);
	void Signup(const FString& UserId, const FString& Password, const FString& Nickname);

	// 로그인 요청
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void RequestLogin(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password);

	// 회원가입 요청
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void RequestSignup(const FString& ServerIP, int32 Port, const FString& UserId, const FString& Password, const FString& Nickname);

private:
	// 서버와의 TCP 소켓 세션 관리
	FSocket* ConnectionSocket;

	// 비동기로 서버 응답 패킷을 수신하기 위한 스레드/타이머 풀링용 함수
	FTimerHandle SocketPollTimerHandle;
	void PollSocketData();

	// 데이터 송수신 헬퍼 함수
	bool ConnectToServer(const FString& ServerIP, int32 Port);
	void CloseConnection();
	bool SendJsonString(const FString& JsonString);
	void ProcessReceivedPacket(const FString& JsonData);
};
