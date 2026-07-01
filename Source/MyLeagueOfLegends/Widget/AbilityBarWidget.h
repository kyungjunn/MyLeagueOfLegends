// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityBarWidget.generated.h"

class USkillComponent;
class UAbilitySlotWidget;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UAbilityBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "Skill UI")
	void InitializeAbilityBar(USkillComponent* PlayerSkillComponent);

protected:
	UPROPERTY(meta = (BindWidget))
	UAbilitySlotWidget* AbilitySlot_Passive;

	UPROPERTY(meta = (BindWidget))
	UAbilitySlotWidget* AbilitySlot_Q;

	UPROPERTY(meta = (BindWidget))
	UAbilitySlotWidget* AbilitySlot_W;

	UPROPERTY(meta = (BindWidget))
	UAbilitySlotWidget* AbilitySlot_E;

	UPROPERTY(meta = (BindWidget))
	UAbilitySlotWidget* AbilitySlot_R;

};
