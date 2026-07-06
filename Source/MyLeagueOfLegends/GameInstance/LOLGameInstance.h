// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "InGame/LOLPlayerState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "LOLGameInstance.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoomListUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinRoomFailed, const FString&, Reason);

// 방 목록 UI에 뿌리기 위한 최소 정보
USTRUCT(BlueprintType)
struct FRoomInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString RoomName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SearchResultIndex = -1;
};


/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ULOLGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	// ================= 계정 정보 =================
	UPROPERTY(BlueprintReadOnly, Category = "Account")
	FString UserId;

	UPROPERTY(BlueprintReadOnly, Category = "Account")
	FString UserNickname;


	// ================= 챔피언 선택 -> 인게임 전달용 =================
	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	FName SavedSelectedChampion = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	ETeam SavedTeam = ETeam::None;

	// ================= 방(세션) 관리 =================
	// 방장이 "방 만들기"를 눌렀을 때 호출 (L_Lobby?listen 으로 이동)
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateRoom(const FString& RoomName, int32 MaxPlayers = 2);

	// 방 목록 새로고침
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindRooms();

	// 방 목록에서 특정 인덱스를 골라 참가
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinRoomByIndex(int32 SearchResultIndex);

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	TArray<FRoomInfo> RoomList;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnRoomListUpdated OnRoomListUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnJoinRoomFailed OnJoinRoomFailed;

private:
	FString PendingLoginId;

	IOnlineSessionPtr GetSessionInterface() const;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;

	static const FName RoomNameSettingsKey; // 세션에 방 이름을 저장할 커스텀 키
};
