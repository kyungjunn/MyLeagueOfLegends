// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class UChampionSelectWidget;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UChampionSelectWidget> ChampionSelectWidgetClass;

	UPROPERTY()
	TObjectPtr<UChampionSelectWidget> ChampionSelectWidgetInstance;
};
