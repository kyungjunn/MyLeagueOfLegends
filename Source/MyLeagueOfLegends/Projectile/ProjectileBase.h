// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UProjectileMovementComponent;

UCLASS(Abstract)
class MYLEAGUEOFLEGENDS_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ¼Ò¸ê ÀýÂ÷ Ã³¸® ÇïÆÛ ÇÔ¼ö
	void StartDestroySequence();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile_Settings")
	float DestroyDelay = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile_Settings")
	float MoveSpeed = 800.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Projectile_Settings", meta = (ExposeOnSpawn = "true"))
	AActor* Attacker;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile Settings", meta = (ExposeOnSpawn = "true"))
	float ProjectileDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile_Settings", meta = (ExposeOnSpawn = "true"))
	TSubclassOf<class UDamageType> ProjectileDamageType;

	UFUNCTION()
	virtual void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


};
