// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "InGame/LOLPlayerState.h"
#include "LOLGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ULOLGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	FName SavedSelectedChampion = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	ETeam SavedTeam = ETeam::None;
};
