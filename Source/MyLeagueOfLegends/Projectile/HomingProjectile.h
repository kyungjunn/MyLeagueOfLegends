// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "HomingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API AHomingProjectile : public AProjectileBase
{
	GENERATED_BODY()
	
public:
	AHomingProjectile();

protected:
	virtual void BeginPlay() override;

	virtual void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Homing", meta = (ExposeOnSpawn = "true"))
	AActor* TargetToAttack;
};
