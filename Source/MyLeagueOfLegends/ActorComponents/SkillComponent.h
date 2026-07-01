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
	Passive, Q, W, E, R
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYLEAGUEOFLEGENDS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 호출할 클라이언트 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void RequestUseSkill(ESkillType SkillType, const FVector& TargetLocation);


	// 쿨타임인지 확인하는 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool IsSkillOnCooldown(ESkillType SkillType);

	// 남은 쿨타임 초 반환 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetSkillRemainingCooldown(ESkillType SkillType);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Slots")
	TMap<ESkillType, USkillDataAsset*> SkillSlots;

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
	// 논타겟팅 발사체 메커니즘 스킬 함수(이즈 Q, 이즈 R, 아리 Q 등 전부 가능)
	void ExecuteNonTargetSkill(class USkillDataAsset* SkillData, const FVector& TargetLocation);

	// 타겟팅 즉시 시전 메커니즘 스킬 함수 (치유, 버프 등)
	void ExecuteTargetSkill(class USkillDataAsset* SkillData, const FVector& TargetLocation);

	// 위치 지정 대시/이동 메커니즘 스킬 함수 (이즈 E, 비점멸 이동 등)
	void ExecuteDashSkill(class USkillDataAsset* SkillData, const FVector& TargetLocation);

protected:
	// 스킬별로 언제 쿨타임이 끝나는지 기록
	UPROPERTY(Replicated)
	float Q_CooldownEndTime;

	UPROPERTY(Replicated)
	float W_CooldownEndTime;

	UPROPERTY(Replicated)
	float E_CooldownEndTime;

	UPROPERTY(Replicated)
	float R_CooldownEndTime;
};
