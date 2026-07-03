// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "InGame/LOLPlayerState.h"
#include "Lobby/LobbyPlayerController.h"

ALobbyGameMode::ALobbyGameMode()
{
	PlayerStateClass = ALOLPlayerState::StaticClass();
	PlayerControllerClass = ALobbyPlayerController::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
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

void ALobbyGameMode::CheckAllPlayersReady()
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

	if (TotalPlayers >= 2 && ReadyCount == TotalPlayers)
	{
		// 하드 트래블 (기본값 bUseSeamlessTravel = false) -> 로딩화면 거쳐 이동
		GetWorld()->ServerTravel(InGameLevelName);
	}
}

