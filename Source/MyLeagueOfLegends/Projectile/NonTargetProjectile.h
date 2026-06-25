// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "NonTargetProjectile.generated.h"

class SkillDataAsset;
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

	// 외부에서 사거리 주입위한 Setter 함수
	FORCEINLINE void SetMaxRange(float InMaxRange) { MaxRange = InMaxRange; }

protected:

	// 사거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (ExposeSpawn))
	float MaxRange = 600.0f; // 기본값

	// 관통 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (ExposeSpawn))
	bool bIsPenetarting = false;

	// 생성 위치
	FVector SpawnLocation;
	
private:
	// 이미 데미지를 입은 타겟들을 저장해두는 배열 (중복 히트 방지)
	UPROPERTY()
	TArray<AActor*> DamagedActors;
};
