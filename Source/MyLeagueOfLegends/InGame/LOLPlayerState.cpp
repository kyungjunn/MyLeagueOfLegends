// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "LOLGameMode.h"
#include "Lobby/LobbyGameMode.h"
#include "GameInstance/LOLGameInstance.h"

ALOLPlayerState::ALOLPlayerState()
{
	Team = ETeam::None;
	SelectedChampion = NAME_None;
	bIsReady = false;
	bAlwaysRelevant = true; // 멀티 UI 동기화 안정성
	Gold = InitialGold;
}

void ALOLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALOLPlayerState, Team);
	DOREPLIFETIME(ALOLPlayerState, SelectedChampion);
	DOREPLIFETIME(ALOLPlayerState, bIsReady);
	DOREPLIFETIME(ALOLPlayerState, Gold);
}

void ALOLPlayerState::C2S_SelectChampion_Implementation(FName ChampionRowName)
{
	if (bIsReady) // 레디 박았으면 변경 불가
	{
		return;
	}

	SelectedChampion = ChampionRowName;
	OnRep_SelectedChampion();
}

void ALOLPlayerState::Server_ReadyUp_Implementation()
{
	if (SelectedChampion.IsNone()) // 캐릭터 안고르면 레디 불가
	{
		return;
	}
	bIsReady = true;

	if (ALobbyGameMode* LobbyGM = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		LobbyGM->CheckAllPlayersReady();
	}
}

void ALOLPlayerState::OnRep_Team()
{
	OnSelectionChanged.Broadcast();
	SaveToLocalGameInstance();
}

void ALOLPlayerState::OnRep_SelectedChampion()
{
	OnSelectionChanged.Broadcast();
	SaveToLocalGameInstance();
}

void ALOLPlayerState::SaveToLocalGameInstance()
{
	// 이 PlayerState가 "내 컴퓨터의 내 캐릭터" 것인지 확인
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || LocalPC->PlayerState != this)
	{
		return; // 상대방 PlayerState면 무시
	}

	if (ULOLGameInstance* GI = GetWorld()->GetGameInstance<ULOLGameInstance>())
	{
		GI->SavedTeam = Team;
		GI->SavedSelectedChampion = SelectedChampion;
	}
}

void ALOLPlayerState::OnRep_Gold()
{
	OnGoldChanged.Broadcast(Gold);
}

// 서버 전용: 골드 차감. 부족하거나 권한 없으면 false
bool ALOLPlayerState::SpendGold(int32 Amount)
{
	if (!HasAuthority() || Amount < 0 || Gold < Amount)
	{
		return false;
	}
	Gold -= Amount;
	OnRep_Gold(); // 리슨서버 호스트는 OnRep이 안 오므로 직접 호출
	return true;
}

// 서버 전용: 골드 획득/환급
void ALOLPlayerState::AddGold(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}
	Gold += Amount;
	OnRep_Gold();
}
