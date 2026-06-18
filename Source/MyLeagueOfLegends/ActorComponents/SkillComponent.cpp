#include "SkillComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Replicated 활성화
	SetIsReplicatedByDefault(true);
}


void USkillComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

// Client 호출
void USkillComponent::RequestUseSkill(ESkillType SkillType, const FVector& TargetLocation)
{
	// 스킬 사용 시 이동 멈춤
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (Owner && Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->StopActiveMovement();

		if (AController* PC = Owner->GetController())
		{
			PC->StopMovement(); // 컨트롤러 단의 이동 명령도 취소
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
	// 서버 검증
	// Stat에서 마나 계산 체크해야함. if (StatComponent->GetCurrentMana() < Q_ManaCost)
	
	// 이동 멈춤
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (Owner && Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->StopActiveMovement();
		if (AController* OwnerController = Owner->GetController())
		{
			OwnerController->StopMovement();
		}
	}

	// 검증 후 스킬 분기
	switch (SkillType)
	{
	case ESkillType::Q:
		ExecuteSkillQ(TargetLocation);
		break;
		//.. 
	}

	// 멀티캐스트 애니메이션, 이펙트
	S2M_SKillEffect(SkillType);
}

void USkillComponent::ExecuteSkillQ(const FVector& TargetLocation)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner || !QProjectileClass)
	{
		return;
	}
	
	// 발사체 스폰 위치
	FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 50.0f;

	// 마우스 커서 방향을 바라보는 회전값
	FRotator SpawnRotation = (TargetLocation - SpawnLocation).Rotation();
	SpawnRotation.Pitch = 0.0f; // 피치 고정(바닥과 평행)

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;	
	SpawnParams.Instigator = Owner;

	// 발사체 스폰
	// 무조건 강제 스폰 처리
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(QProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
}


// 멀티캐스트 - 애니메이션, 이펙트
void USkillComponent::S2M_SKillEffect_Implementation(ESkillType SkillType)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		return;
	}

	if (SkillType == ESkillType::Q && QSkillMontage)
	{
		Owner->PlayAnimMontage(QSkillMontage);
	}
}




