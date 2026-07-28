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

// �� ��� UI�� �Ѹ��� ���� �ּ� ����
USTRUCT(BlueprintType)
struct FRoomInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString RoomName;

	UPROPERTY(BlueprintReadOnly)
	FString HostNickname;

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

	// ================= ���� ���� =================
	UPROPERTY(BlueprintReadOnly, Category = "Account")
	FString UserId;

	UPROPERTY(BlueprintReadOnly, Category = "Account")
	FString UserNickname;


	// ================= è�Ǿ� ���� -> �ΰ��� ���޿� =================
	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	FName SavedSelectedChampion = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	ETeam SavedTeam = ETeam::None;

	// ================= ��(����) ���� =================
	// ������ "�� �����"�� ������ �� ȣ�� (L_Lobby?listen ���� �̵�)
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateRoom(const FString& RoomName, int32 MaxPlayers = 2);

	// �� ��� ���ΰ�ħ
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindRooms();

	// �� ��Ͽ��� Ư�� �ε����� ��� ����
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinRoomByIndex(int32 SearchResultIndex);

	// Reads RoomName back out of the current session (NAME_GameSession) settings.
	// Used to know which chat room to join at Champion Select. Works for both the
	// host and joiners, since RoomName is advertised on the session.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool GetCurrentRoomName(FString& OutRoomName) const;

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

	static const FName RoomNameSettingsKey; // ���ǿ� �� �̸��� ������ Ŀ���� Ű
	static const FName HostNicknameSettingsKey;
};
