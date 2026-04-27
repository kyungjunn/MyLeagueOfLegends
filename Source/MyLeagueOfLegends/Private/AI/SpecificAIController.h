// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SpecificAIController.generated.h"

/**
 * 
 */
UCLASS()
class ASpecificAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ASpecificAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	UPROPERTY(EditAnywhere)
	float CollisionQueryRange;

	void BeginPlay() override;
};
