

#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 동기화 켜기
	bReplicates = true;
	SetReplicateMovement(true);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = Sphere;

	Sphere->SetCollisionProfileName(TEXT("Custom"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	ProjectileDamage = 0.0f;

	ProjectileDamageType = UDamageType::StaticClass();
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnComponentBeginOverlap);
}

// Projectile 액터가 사라질 때 헬퍼 함수
void AProjectileBase::StartDestroySequence()
{
	// 콜리전 끄기
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 이펙트 끄기
	if (Niagara)
	{
		Niagara->Deactivate();
	}

	// 서버에서 소멸
	if (HasAuthority())
	{
		SetLifeSpan(DestroyDelay);
	}
}



void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void AProjectileBase::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}



