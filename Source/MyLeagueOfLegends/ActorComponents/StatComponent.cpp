// Fill out your copyright notice in the Description page of Project Settings.


#include "StatComponent.h"
#include "Net/UnrealNetwork.h" // Replicated 

// Sets default values for this component's properties
UStatComponent::UStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false; // 스탯은 매 프레임 틱 필요 X

	SetIsReplicatedByDefault(true); // 리플리케이션 설정

	// 기본값 초기화
	CharacterRowName = NAME_None;
	StatDataTable = nullptr;
}

void UStatComponent::AddItemBonusStats(float InAD, float InAP, float InDefense, float InAPDefense, float InMaxHP, float InMaxMP, float InMoveSpeed, float InAttackRate)
{
	// 서버에서만 변경
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	MaxHP += InMaxHP;
	CurrentHP = FMath::Clamp(CurrentHP + InMaxHP, 0.0f, MaxHP); // 최대 체력이 늘어난 만큼 현재 체력도 증가

	MaxMP += InMaxMP;
	CurrentMP = FMath::Clamp(CurrentMP + InMaxMP, 0.0f, MaxMP);

	AD += InAD;
	AP += InAP;
	Defense += InDefense;
	APDefense += InAPDefense;
	MoveSpeed += InMoveSpeed;
	AttackRate += InAttackRate;

	if (OnCurrentHPChanged.IsBound()) OnCurrentHPChanged.Broadcast(CurrentHP); 
	if (OnMaxHPChanged.IsBound()) OnMaxHPChanged.Broadcast(MaxHP); 
	if (OnCurrentMPChanged.IsBound()) OnCurrentMPChanged.Broadcast(CurrentMP); 
	if (OnMaxMPChanged.IsBound()) OnMaxMPChanged.Broadcast(MaxMP); 
	if (OnADChanged.IsBound()) OnADChanged.Broadcast(AD); 
	if (OnAPChanged.IsBound()) OnAPChanged.Broadcast(AP); 
	if (OnDefenseChanged.IsBound()) OnDefenseChanged.Broadcast(Defense); 
	if (OnAPDefenseChanged.IsBound()) OnAPDefenseChanged.Broadcast(APDefense); 
	if (OnMoveSpeedChanged.IsBound()) OnMoveSpeedChanged.Broadcast(MoveSpeed); 
	if (OnAttackRateChanged.IsBound()) OnAttackRateChanged.Broadcast(AttackRate);
}

void UStatComponent::RemoveItemBonusStats(float InAD, float InAP, float InDefense, float InAPDefense, float InMaxHP, float InMaxMP, float InMoveSpeed, float InAttackRate)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// 아이템을 팔거나 해제했을 때는 반대로 차감
	MaxHP = FMath::Max(0.0f, MaxHP - InMaxHP);
	CurrentHP = FMath::Clamp(CurrentHP, 0.0f, MaxHP);

	MaxMP = FMath::Max(0.0f, MaxMP - InMaxMP);
	CurrentMP = FMath::Clamp(CurrentMP, 0.0f, MaxMP);

	AD = FMath::Max(0.0f, AD - InAD);
	AP = FMath::Max(0.0f, AP - InAP);
	Defense = FMath::Max(0.0f, Defense - InDefense);
	APDefense = FMath::Max(0.0f, APDefense - InAPDefense);
	MoveSpeed = FMath::Max(0.0f, MoveSpeed - InMoveSpeed);
	AttackRate = FMath::Max(0.0f, AttackRate - InAttackRate);

	// 서버 수동 브로드캐스트
	if (OnMaxHPChanged.IsBound()) OnMaxHPChanged.Broadcast(MaxHP);
	if (OnCurrentHPChanged.IsBound()) OnCurrentHPChanged.Broadcast(CurrentHP);
	if (OnMaxMPChanged.IsBound()) OnMaxMPChanged.Broadcast(MaxMP);
	if (OnCurrentMPChanged.IsBound()) OnCurrentMPChanged.Broadcast(CurrentMP);
	if (OnADChanged.IsBound()) OnADChanged.Broadcast(AD);
	if (OnAPChanged.IsBound()) OnAPChanged.Broadcast(AP);
	if (OnDefenseChanged.IsBound()) OnDefenseChanged.Broadcast(Defense);
	if (OnAPDefenseChanged.IsBound()) OnAPDefenseChanged.Broadcast(APDefense);
	if (OnMoveSpeedChanged.IsBound()) OnMoveSpeedChanged.Broadcast(MoveSpeed);
	if (OnAttackRateChanged.IsBound()) OnAttackRateChanged.Broadcast(AttackRate);
}


// Called when the game starts
void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 서버에서만 데이터테이블을 읽고 스탯 설정하도록.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (StatDataTable != nullptr && CharacterRowName != NAME_None)
		{
			// 데이터테이블에서 RowName으로 행 찾기
			FChampionStatRow* StatRow = StatDataTable->FindRow<FChampionStatRow>(CharacterRowName, TEXT("StatSetup"));

			if (StatRow)
			{
				MaxHP = StatRow->MaxHP;
				CurrentHP = MaxHP;

				MaxMP = StatRow->MaxMP;
				CurrentMP = MaxMP;

				AD = StatRow->AD;
				AP = StatRow->AP;
				Defense = StatRow->Defense;
				APDefense = StatRow->APDefense;
				MoveSpeed = StatRow->MoveSpeed;
				AttackRate = StatRow->AttackRate;

				UE_LOG(LogTemp, Warning, TEXT("[%s] Stat Initialized Successfully! AD: %f, HP: %f"),
					*StatRow->CharacterName.ToString(), AD, MaxHP);

				// 데이터 테이블 로드 끝 알림
				if (OnStatInitComplete.IsBound())
				{
					OnStatInitComplete.Broadcast();
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("StatComponent: Failed to find Row [%s] in DataTable!"), *CharacterRowName.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("StatComponent: DataTable or CharacterRowName is Missing!"));
		}
	}
	
}

void UStatComponent::OnRep_CurrentHP() { if (OnCurrentHPChanged.IsBound()) OnCurrentHPChanged.Broadcast(CurrentHP); }
void UStatComponent::OnRep_MaxHP() { if (OnMaxHPChanged.IsBound()) OnMaxHPChanged.Broadcast(MaxHP); }
void UStatComponent::OnRep_CurrentMP() { if (OnCurrentMPChanged.IsBound()) OnCurrentMPChanged.Broadcast(CurrentMP); }
void UStatComponent::OnRep_MaxMP() { if (OnMaxMPChanged.IsBound()) OnMaxMPChanged.Broadcast(MaxMP); }
void UStatComponent::OnRep_AD() { if (OnADChanged.IsBound()) OnADChanged.Broadcast(AD); }
void UStatComponent::OnRep_AP() { if (OnAPChanged.IsBound()) OnAPChanged.Broadcast(AP); }
void UStatComponent::OnRep_Defense() { if (OnDefenseChanged.IsBound()) OnDefenseChanged.Broadcast(Defense); }
void UStatComponent::OnRep_APDefense() { if (OnAPDefenseChanged.IsBound()) OnAPDefenseChanged.Broadcast(APDefense); }
void UStatComponent::OnRep_MoveSpeed() { if (OnMoveSpeedChanged.IsBound()) OnMoveSpeedChanged.Broadcast(MoveSpeed); }
void UStatComponent::OnRep_AttackRate() { if (OnAttackRateChanged.IsBound()) OnAttackRateChanged.Broadcast(AttackRate); }


// Called every frame
void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatComponent, CurrentHP);
	DOREPLIFETIME(UStatComponent, MaxHP);
	DOREPLIFETIME(UStatComponent, CurrentMP);
	DOREPLIFETIME(UStatComponent, MaxMP);
	DOREPLIFETIME(UStatComponent, AD);
	DOREPLIFETIME(UStatComponent, AP);
	DOREPLIFETIME(UStatComponent, Defense);
	DOREPLIFETIME(UStatComponent, APDefense);
	DOREPLIFETIME(UStatComponent, MoveSpeed);
	DOREPLIFETIME(UStatComponent, AttackRate);
}

