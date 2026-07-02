// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLGameMode.h"
#include "LOLPlayerState.h"
#include "GameFramework/PlayerController.h"

ALOLGameMode::ALOLGameMode()
{
	// 맵 접속 시 캐릭터를 주지 않고 관전자 모드로 대기.
	bStartPlayersAsSpectators = true; 
	PlayerStateClass = ALOLPlayerState::StaticClass();
}

void ALOLGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ALOLPlayerState* PS = NewPlayer->GetPlayerState<ALOLPlayerState>();
	if (PS)
	{
		if (GetNumPlayers() == 1)
		{
			PS->SetTeam(ETeam::Blue);
		}
		else
		{
			PS->SetTeam(ETeam::Red);
		}
	}
}

void ALOLGameMode::CheckAllPlayersReady()
{
	int32 ReadyCount = 0;
	int32 TotalPlayers = GetNumPlayers();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (ALOLPlayerState* PS = PC->GetPlayerState<ALOLPlayerState>())
			{
				if (PS->IsReady())
				{
					ReadyCount++;
				}
			}
		}
	}

	// 1대1 기준 
	if (TotalPlayers >= 2 && ReadyCount == TotalPlayers)
	{
		StartInGameMatch();
	}
}

void ALOLGameMode::StartInGameMatch()
{
	OnInGameMatchStarted.Broadcast();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		ALOLPlayerState* PS = PC->GetPlayerState<ALOLPlayerState>();
		if (!PS)
		{
			continue;
		}
	}
}
