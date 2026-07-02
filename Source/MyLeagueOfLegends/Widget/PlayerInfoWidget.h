// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInfoWidget.generated.h"

class UTextBlock;
class UStatComponent;
class UImage;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UPlayerInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// 메인 HUD나 컨트롤러가 위젯을 생성한 뒤 이즈리얼의 스탯 컴포넌트를 이 위젯 주입(초기화)해 줄 때 사용
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeStatComponent(UStatComponent* InStatComponent);



protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ADText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> APText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MoveSpeedText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackRateText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DefenseText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> APDefenseText;

	UFUNCTION()
	void RefreshAllTexts();

	UFUNCTION() void UpdateADText(float NewValue);
	UFUNCTION() void UpdateAPText(float NewValue);
	UFUNCTION() void UpdateDefenseText(float NewValue);
	UFUNCTION() void UpdateAPDefenseText(float NewValue);
	UFUNCTION() void UpdateMoveSpeedText(float NewValue);
	UFUNCTION() void UpdateAttackRateText(float NewValue);

private:
	UPROPERTY()
	UStatComponent* TargetStatComponent;
};
