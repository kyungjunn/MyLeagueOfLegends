// Fill out your copyright notice in the Description page of Project Settings.


#include "SpecificAIController.h"
#include "Navigation/CrowdFollowingComponent.h"

// 생성자 초기화
ASpecificAIController::ASpecificAIController(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{

}

void ASpecificAIController::BeginPlay()
{
	Super::BeginPlay();

	UCrowdFollowingComponent* CrowdFollowingComponent = FindComponentByClass<UCrowdFollowingComponent>();
	CrowdFollowingComponent->SetCrowdCollisionQueryRange(CollisionQueryRange);
}
