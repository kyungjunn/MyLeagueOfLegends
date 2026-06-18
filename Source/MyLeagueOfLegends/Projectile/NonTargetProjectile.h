// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "NonTargetProjectile.generated.h"

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ANonTargetProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	ANonTargetProjectile();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

	

protected:

	// 사거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (ExposeSpawn))
	float MaxRange = 600.0f;

	// 생성 위치
	FVector SpawnLocation;
	
};
