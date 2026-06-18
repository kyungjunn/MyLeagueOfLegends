// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

class USkillDataAsset;

// 스킬 구분
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Q, W, E, R
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYLEAGUEOFLEGENDS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 호출할 클라이언트 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void RequestUseSkill(ESkillType SkillType, const FVector& TargetLocation);

protected:
	// 서버 RPC 스킬 
	UFUNCTION(Server, Reliable, WithValidation)
	void C2S_UseSkill(ESkillType SkillType, const FVector& TargetLocation);
	void C2S_UseSkill_Implementation(ESkillType SkillType, const FVector& TargetLocation);

	// 멀티캐스트 RPC 애니메이션, 이펙트
	UFUNCTION(NetMulticast, Unreliable)
	void S2M_SKillEffect(ESkillType SkillType);
	void S2M_SKillEffect_Implementation(ESkillType SkillType);

private:
	// 실제 스킬 실행 로직
	void ExecuteSkillQ(const FVector& TargetLocation);
	//void ExecuteSkillW(const FVector& TargetLocation);
	//void ExecuteSkillE(const FVector& TargetLocation);
	//void ExecuteSkillR(const FVector& TargetLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Q")
	TSubclassOf<class AActor> QProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Q")
	class UAnimMontage* QSkillMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Slots")
	TMap<ESkillType, USkillDataAsset*> SkillSlots;
};
