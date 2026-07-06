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

	UFUNCTION(Server, Reliable, Category = "Account")
	void Server_SetNickname(const FString& Nickname);
	void Server_SetNickname_Implementat(const FString& Nickname);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UChampionSelectWidget> ChampionSelectWidgetClass;

	UPROPERTY()
	TObjectPtr<UChampionSelectWidget> ChampionSelectWidgetInstance;
};
