// Fill out your copyright notice in the Description page of Project Settings.


#include "NonTargetProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interface/Damageable.h"
#include "Kismet/GameplayStatics.h"

ANonTargetProjectile::ANonTargetProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANonTargetProjectile::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	AActor* MyOwner = GetOwner();
	APawn* MyPawn = GetInstigator();

	if (Sphere)
	{
		if (MyOwner)
		{
			Sphere->IgnoreActorWhenMoving(MyOwner, true);
		}
		if (MyPawn)
		{
			Sphere->IgnoreActorWhenMoving(MyPawn, true);
		}
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 500.0f;
		ProjectileMovement->MaxSpeed = 500.0f;
		ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
	}
}

void ANonTargetProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float TraveledDistance = FVector::Dist(SpawnLocation, GetActorLocation());

	// 사거리만큼
	if (TraveledDistance >= MaxRange)
	{
		if (ProjectileMovement)
		{
			ProjectileMovement->StopMovementImmediately();
		}
		Destroy();
	}

}

void ANonTargetProjectile::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || OtherActor == Attacker)
	{
		return;
	}

	if (OtherActor->Implements<UDamageable>())
	{
		AController* InstigatorController = nullptr;
		if (APawn* AttackerPawn = Cast<APawn>(Attacker))
		{
			InstigatorController = AttackerPawn->GetController();
		}

		UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, InstigatorController, Attacker, ProjectileDamageType);

		if (ProjectileMovement)
		{
			ProjectileMovement->StopMovementImmediately();
		}

		Destroy();
	}
}
