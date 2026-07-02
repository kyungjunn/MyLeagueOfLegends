// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LOLGameMode.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInGameMatchStarted);

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ALOLGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALOLGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 준비 체크
	void CheckAllPlayersReady();

	// 매치 시작 신호
	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnInGameMatchStarted OnInGameMatchStarted;

protected:
	// 챔 선택 이후 게임 시작
	void StartInGameMatch();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	UDataTable* ChampionDataTable;
};
