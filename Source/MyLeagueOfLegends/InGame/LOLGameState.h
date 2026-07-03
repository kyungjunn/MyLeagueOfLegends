// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LOLGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInGameMatchStarted);

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ALOLGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_MatchStarted, BlueprintReadOnly)
	bool bMatchStarted = false;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnInGameMatchStarted OnInGameMatchStarted;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_MatchStarted();
};
