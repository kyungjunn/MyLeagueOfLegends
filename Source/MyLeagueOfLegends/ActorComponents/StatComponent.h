// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Engine/DataTable.h" // 데이터 테이블 사용

#include "StatComponent.generated.h"

class UTexture2D;

// 스탯 로드 완료 시 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatInitCompleteDelegate);

// 스탯 변경 시 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChangedDelegate, float, NewStatValue);

USTRUCT(BlueprintType)
// 데이터 테이블과 매칭되는 구조체 선언
struct FChampionStatRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FName CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	TObjectPtr<UTexture2D> CharacterIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<APawn> ChampionPawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Defense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float APDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackRate;
};

UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYLEAGUEOFLEGENDS_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatComponent();

	// 스탯 로드 완료 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatInitCompleteDelegate OnStatInitComplete;

	// 아이템 구매/장착 시 스탯을 가산해주는 함수 (블루프린트 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Stat|Adjustment")
	void AddItemBonusStats(float InAD, float InAP, float InDefense, float InAPDefense, float InMaxHP, float InMaxMP, float InMoveSpeed, float InAttackRate);

	UFUNCTION(BlueprintCallable, Category = "Stat|Adjustment")
	void RemoveItemBonusStats(float InAD, float InAP, float InDefense, float InAPDefense, float InMaxHP, float InMaxMP, float InMoveSpeed, float InAttackRate);

	// UI 델리게이트 
	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnCurrentHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnMaxHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnCurrentMPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnMaxMPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnADChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnAPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnDefenseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnAPDefenseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnMoveSpeedChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Stat")
	FOnStatChangedDelegate OnAttackRateChanged;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION() void OnRep_CurrentHP();

	UFUNCTION() void OnRep_MaxHP();

	UFUNCTION() void OnRep_CurrentMP();

	UFUNCTION() void OnRep_MaxMP();

	UFUNCTION()void OnRep_AD();

	UFUNCTION() void OnRep_AP();

	UFUNCTION() void OnRep_Defense();

	UFUNCTION() void OnRep_APDefense();

	UFUNCTION() void OnRep_MoveSpeed();

	UFUNCTION() void OnRep_AttackRate();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 멀티플레이어 변수 동기화를 위한 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	FName CharacterRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	UDataTable* StatDataTable;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TObjectPtr<UTexture2D> CharacterIcon;

	// ReplicatedUsing 매크로 연동
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentHP;

	UPROPERTY(ReplicatedUsing = OnRep_MaxHP, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float MaxHP;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMP, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentMP;

	UPROPERTY(ReplicatedUsing = OnRep_MaxMP, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float MaxMP;

	UPROPERTY(ReplicatedUsing = OnRep_AD, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float AD;

	UPROPERTY(ReplicatedUsing = OnRep_AP, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float AP;

	UPROPERTY(ReplicatedUsing = OnRep_Defense, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float Defense;

	UPROPERTY(ReplicatedUsing = OnRep_APDefense, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float APDefense;

	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float MoveSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_AttackRate, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float AttackRate;

public:
	// 외부 안전 접근용 Getter 진형
	FORCEINLINE UTexture2D* GetCharacterIcon() const { return CharacterIcon; }
	FORCEINLINE float GetCurrentHP() const { return CurrentHP; }
	FORCEINLINE float GetMaxHP() const { return MaxHP; }
	FORCEINLINE float GetCurrentMP() const { return CurrentMP; }
	FORCEINLINE float GetMaxMP() const { return MaxMP; }
	FORCEINLINE float GetAttackDamage() const { return AD; }
	FORCEINLINE float GetAbilityPower() const { return AP; }
	FORCEINLINE float GetDefense() const { return Defense; }
	FORCEINLINE float GetAPDefense() const { return APDefense; }
	FORCEINLINE float GetMoveSpeed() const { return MoveSpeed; }
	FORCEINLINE float GetAttackRate() const { return AttackRate; }
};
