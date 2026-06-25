// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillDataAsset.generated.h"


class UTexture2D;


// 스킬 메커니즘 타입
UENUM(BlueprintType)
enum class ESkillMechanicType : uint8
{
	NonTargetProjectile		UMETA(DisplayName = "NonTarget Projectile"),
	TargetProjectile		UMETA(DisplayName = "Targeting Instant"),
	LocationDash			UMETA(DisplayName = "Location Dash")
};


/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API USkillDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info")
	FString SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info")
	UTexture2D* SkillIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info")
	ESkillMechanicType MechanicType;

	// 스킬 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Stats")
	float CoolDown = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Stats")
	float Cost = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Stats")
	float BaseDamage = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Stats")
	float MaxRange = 300.0f;

	// 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	class UAnimMontage* SkillMontage;
	
	// 발사체
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;
};
