// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"

AHomingProjectile::AHomingProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHomingProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(TargetToAttack))
	{
		InitializeHoming();
	}
	
}

void AHomingProjectile::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && OtherActor == TargetToAttack)
	{
		AController* InstigatorController = nullptr;
		if (APawn* AttackerPawn = Cast<APawn>(Attacker))
		{
			InstigatorController = AttackerPawn->GetController();
		}

		UGameplayStatics::ApplyDamage(TargetToAttack, ProjectileDamage, InstigatorController, Attacker, ProjectileDamageType);

		// 서버에서 즉시 파괴
		//UE_LOG(LogTemp, Warning, TEXT("Server Destroy"));
		Destroy();
	}
}

void AHomingProjectile::OnRep_TargetToAttack()
{
	InitializeHoming();
}

void AHomingProjectile::InitializeHoming()
{
	if (ProjectileMovement && IsValid(TargetToAttack))
	{
		ProjectileMovement->ProjectileGravityScale = 0.0f;

		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingAccelerationMagnitude = 1000.0f;

		ProjectileMovement->HomingTargetComponent = TargetToAttack->GetRootComponent();
	}
}

void AHomingProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHomingProjectile, TargetToAttack);
}


