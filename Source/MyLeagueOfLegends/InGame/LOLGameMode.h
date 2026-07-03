// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LOLPlayerState.h"

#include "LOLGameMode.generated.h"


/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ALOLGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALOLGameMode();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void OnPlayerSubmittedSelection(APlayerController* InPC, ALOLPlayerState* InPS);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	UDataTable* ChampionDataTable;

	void SpawnChampionForPlayer(APlayerController* InPC, ALOLPlayerState* InPS);

	AActor* FindPlayerStartForTeam(ETeam Team);

	int32 SpawnedPlayerCount = 0;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TArray<AActor*> BlueSpawnPoints;
};
