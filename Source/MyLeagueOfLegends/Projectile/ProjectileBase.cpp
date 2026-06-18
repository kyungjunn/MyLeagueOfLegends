

#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Net/UnrealNetwork.h"

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

void AProjectileBase::Destroyed()
{
	UE_LOG(LogTemp, Warning, TEXT("Niagara Destory"));
	Super::Destroyed();

	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (Niagara)
	{
		// 비활성화
		Niagara->Deactivate();
	}
}

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectileBase, ProjectileDamage);
}


void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void AProjectileBase::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}



