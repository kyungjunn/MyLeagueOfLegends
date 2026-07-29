// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLGameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Network/AuthClient.h"
#include "Kismet/GameplayStatics.h"

const FName ULOLGameInstance::RoomNameSettingsKey(TEXT("ROOM_NAME"));
const FName ULOLGameInstance::HostNicknameSettingsKey(TEXT("HOST_NICKNAME"));

void ULOLGameInstance::Init()
{
	Super::Init();
}

// ================= ��(����) =================

IOnlineSessionPtr ULOLGameInstance::GetSessionInterface() const
{
	IOnlineSubsystem* SubSystem = Online::GetSubsystem(GetWorld());
	return SubSystem ? SubSystem->GetSessionInterface() : nullptr;
}

void ULOLGameInstance::CreateRoom(const FString& RoomName, int32 MaxPlayers)
{

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		OnJoinRoomFailed.Broadcast(TEXT("Online subsystem is valid."));
		return;
	}

	// ���� ������ ������ ���� ����
	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		Sessions->DestroySession(NAME_GameSession);
	}

	SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->NumPublicConnections = MaxPlayers;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bIsLANMatch = true; // �¶��� ����ý����� Null�� �� LAN ��ε�ĳ��Ʈ�� �˻� ����
	SessionSettings->bUsesPresence = false;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = false;
	SessionSettings->bIsDedicated = false;
	SessionSettings->Set(RoomNameSettingsKey, RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(HostNicknameSettingsKey, UserNickname, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ULOLGameInstance::OnCreateSessionComplete));

	Sessions->CreateSession(0, NAME_GameSession, *SessionSettings);
}

void ULOLGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
	}

	if (!bWasSuccessful)
	{
		OnJoinRoomFailed.Broadcast(TEXT("Failed to create room."));
		return;
	}

	// ������ ���� ������ L_Lobby ������ ���� ������ ����
	UGameplayStatics::OpenLevel(this, FName(TEXT("Lobby")), true, TEXT("?listen"));
}

void ULOLGameInstance::FindRooms()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		OnJoinRoomFailed.Broadcast(TEXT("Online subsystem is valid."));
		return;
	}
 
	if (SessionSearch.IsValid() && SessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("already session searching"));
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = 50;
	SessionSearch->bIsLanQuery = true;
	SessionSearch->QuerySettings.Set(TEXT("PRESENCE"), false, EOnlineComparisonOp::Equals);

	FindSessionsDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ULOLGameInstance::OnFindSessionsComplete));

	Sessions->FindSessions(0, SessionSearch.ToSharedRef());
}

void ULOLGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
	}

	RoomList.Reset();

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		const TArray<FOnlineSessionSearchResult>& Results = SessionSearch->SearchResults;
		for (int32 i = 0; i < Results.Num(); ++i)
		{
			const FOnlineSessionSearchResult& Result = Results[i];

			// ���ǿ��� �� �̸� ��������
			FString RoomName;
			Result.Session.SessionSettings.Get(RoomNameSettingsKey, RoomName);
			if (RoomName.IsEmpty())
			{
				RoomName = FString::Printf(TEXT("Room %d"), i + 1);
			}

			// ���ǿ��� ȣ��Ʈ �̸� ��������
			FString HostNickname;
			Result.Session.SessionSettings.Get(HostNicknameSettingsKey, HostNickname);
			if (HostNickname.IsEmpty())
			{
				HostNickname = TEXT("Unknown");
			}

			FRoomInfo Info;
			Info.RoomName = RoomName;
			Info.HostNickname = HostNickname;
			Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - Result.Session.NumOpenPublicConnections;
			Info.SearchResultIndex = i;
			RoomList.Add(Info);
		}
	}

	OnRoomListUpdated.Broadcast();
}

void ULOLGameInstance::JoinRoomByIndex(int32 SearchResultIndex)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid() || !SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
	{
		OnJoinRoomFailed.Broadcast(TEXT("Invalid room index."));
		return;
	}

	JoinSessionDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ULOLGameInstance::OnJoinSessionComplete));

	Sessions->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SearchResultIndex]);
}

void ULOLGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid()) return;

	Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		OnJoinRoomFailed.Broadcast(TEXT("Failed to join room."));
		return;
	}

	FString ConnectString;
	if (!Sessions->GetResolvedConnectString(SessionName, ConnectString))
	{
		OnJoinRoomFailed.Broadcast(TEXT("Failed to resolve connect string."));
		return;
	}

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		// ?Name= �ɼ����� �����ϸ� ������ �ڵ����� PlayerState�� PlayerName�� ä���ݴϴ�.
		// (�г��� ����ȭ�� LobbyPlayerController���� �� �� �� ������ Ȯ���մϴ�)
		const FString TravelURL = FString::Printf(TEXT("%s?Name=%s"), *ConnectString, *UserNickname);
		PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
}

bool ULOLGameInstance::GetCurrentRoomName(FString& OutRoomName) const
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions.IsValid())
	{
		return false;
	}

	FNamedOnlineSession* Session = Sessions->GetNamedSession(NAME_GameSession);
	if (!Session)
	{
		return false;
	}

	return Session->SessionSettings.Get(RoomNameSettingsKey, OutRoomName) && !OutRoomName.IsEmpty();
}
