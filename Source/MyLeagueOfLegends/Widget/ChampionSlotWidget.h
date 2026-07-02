// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChampionSlotWidget.generated.h"

class UButton;
class UImage;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UChampionSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeSlot(FName InRowName, UTexture2D* InIcon);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ChampionImage;

private:
	UFUNCTION()
	void OnSlotClicked();

	FName TargetRowName;
};
