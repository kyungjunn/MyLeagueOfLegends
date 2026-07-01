// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySlotWidget.generated.h"

class USkillDataAsset;
class USkillComponent;
class UImage;
class UTextBlock;
class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UAbilitySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "SKill UI")
	void InitSlot(USkillComponent* InSkillComponent, ESkillType InSkillType);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeomerty, float InDeltaTime) override;

	void UpdateSlot();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Icon_Image;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CoolTime_Text;
	
	// 쿨타임용 이미지도 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Cooldown_Image;

private:
	UPROPERTY()
	USkillComponent* TargetSkillComponent;
	
	ESkillType AssignedSkillType;

	UPROPERTY()
	USkillDataAsset* CachedSkillData;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CooldownMaterialDynamic;
};
