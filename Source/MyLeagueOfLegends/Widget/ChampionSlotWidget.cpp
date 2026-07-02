// Fill out your copyright notice in the Description page of Project Settings.


#include "ChampionSlotWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "InGame/LOLPlayerState.h"

void UChampionSlotWidget::InitializeSlot(FName InRowName, UTexture2D* InIcon)
{
	TargetRowName = InRowName;

	if (ChampionImage && InIcon)
	{
		ChampionImage->SetBrushFromTexture(InIcon);
	}
}

void UChampionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UChampionSlotWidget::OnSlotClicked);
	}
}

void UChampionSlotWidget::OnSlotClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ALOLPlayerState* PS = PC->GetPlayerState<ALOLPlayerState>())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Slot Clicked! Requesting: %s"), *TargetRowName.ToString());
			PS->C2S_SelectChampion(TargetRowName);
		}
	}
}
