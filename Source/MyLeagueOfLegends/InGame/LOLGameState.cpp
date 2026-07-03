// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLGameState.h"
#include "Net/UnrealNetwork.h"

void ALOLGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALOLGameState, bMatchStarted);
}

void ALOLGameState::OnRep_MatchStarted()
{
	OnInGameMatchStarted.Broadcast();
}
