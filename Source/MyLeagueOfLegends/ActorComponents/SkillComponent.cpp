#include "SkillComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DataAssets/SkillDataAsset.h"
#include "Projectile/ProjectileBase.h"
#include "Net/UnrealNetwork.h"

#include "Components/CapsuleComponent.h"
#include "Projectile/NonTargetProjectile.h"


USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Replicated 활성화
	SetIsReplicatedByDefault(true);
}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USkillComponent, Q_CooldownEndTime);
	DOREPLIFETIME(USkillComponent, W_CooldownEndTime);
	DOREPLIFETIME(USkillComponent, E_CooldownEndTime);
	DOREPLIFETIME(USkillComponent, R_CooldownEndTime);
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

// 쿨타임 여부 계산
bool USkillComponent::IsSkillOnCooldown(ESkillType SkillType)
{
	float EndTime = 0.0f;
	switch (SkillType)
	{
	case ESkillType::Q: EndTime = Q_CooldownEndTime;
		break;
	case ESkillType::W: EndTime = W_CooldownEndTime; 
		break;
	case ESkillType::E: EndTime = E_CooldownEndTime; 
		break;
	case ESkillType::R: EndTime = R_CooldownEndTime; 
		break;
	}
	return GetWorld()->GetTimeSeconds() < EndTime;
}

// 남은 시간 계산 (UI용)
float USkillComponent::GetSkillRemainingCooldown(ESkillType SkillType)
{
	if (!IsSkillOnCooldown(SkillType))
	{
		return 0.0f;
	}

	switch (SkillType)
	{
	case ESkillType::Q:
		return Q_CooldownEndTime - GetWorld()->GetTimeSeconds();
		break;
	case ESkillType::W:
		return W_CooldownEndTime - GetWorld()->GetTimeSeconds();
		break;
	case ESkillType::E:
		return E_CooldownEndTime - GetWorld()->GetTimeSeconds();
		break;
	case ESkillType::R:
		return R_CooldownEndTime - GetWorld()->GetTimeSeconds();
		break;
	}

	return 0.0f;
}

// Client 호출
void USkillComponent::RequestUseSkill(ESkillType SkillType, const FVector& TargetLocation)
{
	if (IsSkillOnCooldown(SkillType))
	{
		return;
	}

	// 스킬 사용 시 이동 멈춤
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (Owner && Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->bOrientRotationToMovement = false;

		Owner->GetCharacterMovement()->StopActiveMovement();
		if (AController* PC = Owner->GetController())
		{
			PC->StopMovement(); // 컨트롤러 단의 이동 명령도 취소
		}

		FVector LookDirection = (TargetLocation - Owner->GetActorLocation());
		LookDirection.Z = 0.0f;

		if (!LookDirection.IsNearlyZero())
		{
			Owner->SetActorRotation(LookDirection.Rotation());
		}
	}

	// 클라이언트일 때만 예측
	if (Owner && !Owner->HasAuthority())
	{
		// 클라이언트 로컬 반응성을 위해 미리 예측하여 쿨타임 반영
		if (SkillSlots.Contains(SkillType) && SkillSlots[SkillType])
		{
			float PredictEndTime = GetWorld()->GetTimeSeconds() + SkillSlots[SkillType]->CoolDown;
			switch (SkillType)
			{
			case ESkillType::Q: Q_CooldownEndTime = PredictEndTime; break;
			case ESkillType::W: W_CooldownEndTime = PredictEndTime; break;
			case ESkillType::E: E_CooldownEndTime = PredictEndTime; break;
			case ESkillType::R: R_CooldownEndTime = PredictEndTime; break;
			}
		}
	}

	C2S_UseSkill(SkillType, TargetLocation);
}

// 서버 실행
bool USkillComponent::C2S_UseSkill_Validate(ESkillType SkillType, const FVector& TargetLocation)
{
	return true;
}
void USkillComponent::C2S_UseSkill_Implementation(ESkillType SkillType, const FVector& TargetLocation)
{
	if (!SkillSlots.Contains(SkillType) || SkillSlots[SkillType] == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("what???"));
		return;
	}
	// 슬롯에 등록된 데이터 에셋 가져오기
	USkillDataAsset* ActiveSkillData = SkillSlots[SkillType];


	// 쿨타임 적용
	if (IsSkillOnCooldown(SkillType))
	{
		return; // 핵 유저라면 여기서 차단당함
	}

	float NewEndTime = GetWorld()->GetTimeSeconds() + ActiveSkillData->CoolDown;
	switch (SkillType)
	{
	case ESkillType::Q: Q_CooldownEndTime = NewEndTime; break;
	case ESkillType::W: W_CooldownEndTime = NewEndTime; break;
	case ESkillType::E: E_CooldownEndTime = NewEndTime; break;
	case ESkillType::R: R_CooldownEndTime = NewEndTime; break;
	}

	// 서버 검증
	// Stat에서 마나 계산 체크해야함. if (StatComponent->GetCurrentMana() < ActiveSkillData->ManaCost) return;
	
	// 이동 멈춤
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (Owner && Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->StopActiveMovement();
		if (AController* OwnerController = Owner->GetController())
		{
			OwnerController->StopMovement();
		}

		FVector LookDirection = (TargetLocation - Owner->GetActorLocation());
		LookDirection.Z = 0.0f;

		if (!LookDirection.IsNearlyZero())
		{
			Owner->SetActorRotation(LookDirection.Rotation());
		}
	}


	// 검증 후 메커니즘 분기
	switch (ActiveSkillData->MechanicType)
	{
		// 논타겟 발사체
	case ESkillMechanicType::NonTargetProjectile:
		ExecuteNonTargetSkill(ActiveSkillData, TargetLocation);
		break;
		
		// 타겟 발사체
	case ESkillMechanicType::TargetProjectile:
		ExecuteTargetSkill(ActiveSkillData, TargetLocation);
		break;

		// 이동기
	case ESkillMechanicType::LocationDash:
		ExecuteDashSkill(ActiveSkillData, TargetLocation);
		break;

	}

	// 멀티캐스트 애니메이션, 이펙트
	S2M_SKillEffect(SkillType);
}

void USkillComponent::ExecuteNonTargetSkill(USkillDataAsset* SkillData, const FVector& TargetLocation)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	// 데이터 에셋 내부의 발사체 클래스가 유효한지 검사
	if (!Owner || !SkillData || !(SkillData->ProjectileClass))
	{
		return;
	}

	if (Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
		Owner->GetCharacterMovement()->StopActiveMovement();
	}

	// 발사체 스폰 위치
	FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 50.0f;

	// 스킬쓸 때 회전을 하니까 그냥 캐릭터의 방향으로 
	FRotator SpawnRotation = Owner->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Owner;

	// 발사체 강제 스폰 
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 데이터 에셋에 등록되어 있는 발사체 클래스를 스폰
	AActor* SpawnedProjectile = GetWorld()->SpawnActor<AActor>(SkillData->ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (SpawnedProjectile)
	{	
		Owner->MoveIgnoreActorAdd(SpawnedProjectile);
		
		// 데이터 에셋에 등록되어 있는 데미지 수치를 발사체에 세팅
		ANonTargetProjectile* NonTargetProj = Cast<ANonTargetProjectile>(SpawnedProjectile);
		if (NonTargetProj)
		{
			NonTargetProj->SetProjectileDamage(SkillData->BaseDamage);
			NonTargetProj->SetMaxRange(SkillData->MaxRange);
		}
	}
}

void USkillComponent::ExecuteTargetSkill(USkillDataAsset* SkillData, const FVector& TargetLocation)
{
}

void USkillComponent::ExecuteDashSkill(USkillDataAsset* SkillData, const FVector& TargetLocation)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner || !SkillData)
	{
		return;
	}

	// 비전이동은 서버에서 위치를 변경
	if (Owner->HasAuthority() == false)
	{
		return;
	}

	FVector StartLocation = Owner->GetActorLocation();

	// 평면 연산을 위해 높이(Z) 값을 시전자 기준으로 통일
	FVector FlatTarget = TargetLocation;
	FlatTarget.Z = StartLocation.Z;

	// 마우스 클릭 위치까지의 방향과 거리를 계산
	FVector DashDirection = FlatTarget - StartLocation;
	float DistanceToTarget = DashDirection.Size();
	DashDirection.Normalize();

	// 최종적으로 도달할 순간이동 목적지 변수
	FVector FinalDestination = FlatTarget;

	// 데이터 에셋에 지정된 최대 사거리를 넘어서 클릭했다면, 사거리 한계선까지만 이동
	float SkillMaxRange = SkillData->MaxRange > 0.0f ? SkillData->MaxRange : 475.0f; // 디폴트 475cm

	if (DistanceToTarget > SkillMaxRange)
	{
		FinalDestination = StartLocation + (DashDirection * SkillMaxRange);
	}

	// 순간이동한 위치에 있는 땅바닥 높이로 Z축을 보정
	FVector SafeDestination = FinalDestination;
	FHitResult GroundHit;
	FVector CheckStart = FinalDestination + FVector(0.0f, 0.0f, 100.0f);   // 내 머리 위 1m
	FVector CheckEnd = FinalDestination - FVector(0.0f, 0.0f, 500.0f);     // 발밑 5m 탐색

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(Owner);

	// 바닥 지형(Static 월드 콜리전)을 수직 레이캐스팅 트레이스
	if (GetWorld()->LineTraceSingleByChannel(GroundHit, CheckStart, CheckEnd, ECC_WorldStatic, TraceParams))
	{
		SafeDestination.Z = GroundHit.ImpactPoint.Z + Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	// 확인된 위치로 시전자를 텔레포트
	// bNoKnockback=true, bKeepVelocity=false 옵션으로 안전하게 이동
	Owner->SetActorLocation(SafeDestination, false, nullptr, ETeleportType::TeleportPhysics);

	// 비전이동이 완료된 직후, 캐릭터 Mesh가 날아간 목적지 방향을 바라보게 정렬
	if (!DashDirection.IsNearlyZero())
	{
		Owner->SetActorRotation(DashDirection.Rotation());
	}

	// 비전이동 후 원래 가려던 이동 명령 패킷(무브먼트 관성)이 남아있다면 꼬이므로 무브먼트를 지움.
	if (Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->StopActiveMovement();
	}
}



// 멀티캐스트 - 애니메이션, 이펙트
void USkillComponent::S2M_SKillEffect_Implementation(ESkillType SkillType)
{
	if (!SkillSlots.Contains(SkillType) || SkillSlots[SkillType] == nullptr)
	{
		return;
	}

	USkillDataAsset* SkillData = SkillSlots[SkillType];

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		return;
	}

	if (SkillData || SkillData->SkillMontage)
	{
		Owner->PlayAnimMontage(SkillData->SkillMontage);
	}
}