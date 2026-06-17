// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AHomingProjectile::AHomingProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHomingProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileMovement && IsValid(TargetToAttack))
	{
		ProjectileMovement->ProjectileGravityScale = 0.0f;

		ProjectileMovement->Velocity = GetActorForwardVector() * MoveSpeed;

		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingAccelerationMagnitude = 5000.0f;

		ProjectileMovement->HomingTargetComponent = TargetToAttack->GetRootComponent();
	}
	else
	{
		StartDestroySequence();
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

		StartDestroySequence();
	}
}


