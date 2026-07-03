// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLGameMode.h"
#include "LOLPlayerState.h"
#include "LOLGameState.h"
#include "ActorComponents/StatComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

ALOLGameMode::ALOLGameMode()
{

}

void ALOLGameMode::OnPlayerSubmittedSelection(APlayerController* InPC, ALOLPlayerState* InPS)
{
	SpawnChampionForPlayer(InPC, InPS);

	if (++SpawnedPlayerCount >= GetNumPlayers())
	{
		if (ALOLGameState* GS = GetGameState<ALOLGameState>())
		{
			GS->bMatchStarted = true;
			GS->OnRep_MatchStarted();
		}
	}
}


void ALOLGameMode::SpawnChampionForPlayer(APlayerController* InPC, ALOLPlayerState* InPS)
{
	if (!ChampionDataTable)
	{
		return;
	}

	FName RowName = InPS->GetSelectedChampion();
	FChampionStatRow* StatRow = ChampionDataTable->FindRow< FChampionStatRow>(RowName, TEXT(""));
	if (!StatRow || !StatRow->ChampionPawnClass)
	{
		return;
	}

	// 팀에 맞게 스폰위치 
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (AActor* StartSpot = FindPlayerStartForTeam(InPS->GetTeam()))
	{
		SpawnLocation = StartSpot->GetActorLocation();
		SpawnRotation = StartSpot->GetActorRotation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(StatRow->ChampionPawnClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (NewPawn)
	{
		// 기존에 조종 중이던 임시 Pawn(선택 화면용 옵저버 등)이 있다면 정리
		if (APawn* OldPawn = InPC->GetPawn())
		{
			OldPawn->Destroy();
		}

		InPC->Possess(NewPawn);
	}
}

AActor* ALOLGameMode::FindPlayerStartForTeam(ETeam Team)
{
	FString Tag = (Team == ETeam::Blue) ? TEXT("BlueStart") : TEXT("RedStart");
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == FName(*Tag))
		{
			return *It;
		}
	}
	return nullptr;
}
