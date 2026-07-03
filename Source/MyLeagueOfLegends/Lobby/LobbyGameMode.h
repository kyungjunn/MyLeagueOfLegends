// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "LobbyGameMode.generated.h"


class ALOLPlayerState;
/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 준비 체크
	void CheckAllPlayersReady();

protected:
	UPROPERTY(EditAnywhere, Category = "Setup")
	FString InGameLevelName = TEXT("/Game/Maps/InGame");
};
