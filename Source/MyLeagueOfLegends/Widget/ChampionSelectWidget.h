// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChampionSelectWidget.generated.h"

class UWrapBox;
class UButton;
class UImage;
class UChampionSlotWidget;
class UDataTable;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UChampionSelectWidget : public UUserWidget
{
	GENERATED_BODY()

	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> ChampionListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BlueChampionImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RedChampionImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UDataTable> ChampionDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TSubclassOf<UChampionSlotWidget> ChampionSlotClass;

public:
	// PlayerState들의 복제 알림을 받아 UI를 리프레시할 콜백 함수
	UFUNCTION()
	void RefreshSelectionUI();

private:
	UFUNCTION()
	void OnReadyClicked();

	void GenerateChampionList();

	bool bEnemyBound = false;
	bool bSelfBound = false;

};
