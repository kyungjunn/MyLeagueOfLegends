// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLGameMode.h"
#include "LOLPlayerState.h"
#include "LOLGameState.h"
#include "ActorComponents/StatComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"

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

		// 매치 시작 시 초당 패시브 골드 지급 시작 (서버 전용)
		GetWorldTimerManager().SetTimer(GoldIncomeTimerHandle, this, &ALOLGameMode::GrantPassiveGold, GoldTickInterval, true);
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
	const FName Tag = (Team == ETeam::Blue) ? FName(TEXT("BlueStart")) : FName(TEXT("RedStart"));

	// 1순위: 레벨에 배치한 팀 스폰 액터(BP_BlueSpawn / BP_RedSpawn) - 액터 태그로 검색
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(Tag))
		{
			return *It;
		}
	}

	// 2순위: PlayerStartTag 가 설정된 PlayerStart (기존 방식 폴백)
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == Tag)
		{
			return *It;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("FindPlayerStartForTeam: '%s' 스폰 지점을 찾지 못했습니다."), *Tag.ToString());
	return nullptr;
}

// 초당 전원에게 패시브 골드 지급 (GameMode는 서버에만 존재)
void ALOLGameMode::GrantPassiveGold()
{
	AGameStateBase* GS = GetGameState<AGameStateBase>();
	if (!GS)
	{
		return;
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (ALOLPlayerState* LOLPS = Cast<ALOLPlayerState>(PS))
		{
			LOLPS->AddGold(GoldPerTick);
		}
	}
}
