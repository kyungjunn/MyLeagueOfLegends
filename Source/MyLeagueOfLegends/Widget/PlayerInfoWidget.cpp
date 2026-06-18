// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerInfoWidget.h"
#include "Components/TextBlock.h"
#include "ActorComponents/StatComponent.h"


void UPlayerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (OwningPawn)
	{
		UStatComponent* StatComponent = OwningPawn->FindComponentByClass<UStatComponent>();
		if (StatComponent)
		{
			InitializeStatComponent(StatComponent);
			StatComponent->OnStatInitComplete.AddDynamic(this, &UPlayerInfoWidget::RefreshAllTexts);
		}
	}

}

void UPlayerInfoWidget::InitializeStatComponent(UStatComponent* InStatComponent)
{
	{
		if (InStatComponent == nullptr) return;

		TargetStatComponent = InStatComponent;

		// 이벤트 바인딩
		TargetStatComponent->OnADChanged.AddDynamic(this, &UPlayerInfoWidget::UpdateADText);
		TargetStatComponent->OnAPChanged.AddDynamic(this, &UPlayerInfoWidget::UpdateAPText);
		TargetStatComponent->OnDefenseChanged.AddDynamic(this, &UPlayerInfoWidget::UpdateDefenseText);
		TargetStatComponent->OnAPDefenseChanged.AddDynamic(this, &UPlayerInfoWidget::UpdateAPDefenseText);
		TargetStatComponent->OnMoveSpeedChanged.AddDynamic(this, &UPlayerInfoWidget::UpdateMoveSpeedText);
		TargetStatComponent->OnAttackRateChanged.AddDynamic(this, &UPlayerInfoWidget::UpdateAttackRateText);

		// 초기화 시점 데이터 일치
		RefreshAllTexts();
	}
}

void UPlayerInfoWidget::RefreshAllTexts()
{
	UpdateADText(TargetStatComponent->GetAttackDamage());
	UpdateAPText(TargetStatComponent->GetAbilityPower());
	UpdateDefenseText(TargetStatComponent->GetDefense());
	UpdateAPDefenseText(TargetStatComponent->GetAPDefense());
	UpdateMoveSpeedText(TargetStatComponent->GetMoveSpeed());
	UpdateAttackRateText(TargetStatComponent->GetAttackRate());
}

void UPlayerInfoWidget::UpdateADText(float NewValue)
{
	if (ADText) ADText->SetText(FText::AsNumber(FMath::RoundToInt(NewValue))); // AD는 주로 정수로 표현
}

void UPlayerInfoWidget::UpdateAPText(float NewValue)
{
	if (APText) APText->SetText(FText::AsNumber(FMath::RoundToInt(NewValue)));
}

void UPlayerInfoWidget::UpdateDefenseText(float NewValue)
{
	if (DefenseText) DefenseText->SetText(FText::AsNumber(FMath::RoundToInt(NewValue)));
}

void UPlayerInfoWidget::UpdateAPDefenseText(float NewValue)
{
	if (APDefenseText) APDefenseText->SetText(FText::AsNumber(FMath::RoundToInt(NewValue)));
}

void UPlayerInfoWidget::UpdateMoveSpeedText(float NewValue)
{
	if (MoveSpeedText) MoveSpeedText->SetText(FText::AsNumber(FMath::RoundToInt(NewValue)));
}

void UPlayerInfoWidget::UpdateAttackRateText(float NewValue)
{
	if (AttackRateText)
	{
		// 소수점 둘째 자리까지 표현
		FNumberFormattingOptions Options;
		Options.SetMaximumFractionalDigits(2);
		Options.SetMinimumFractionalDigits(2);

		AttackRateText->SetText(FText::AsNumber(NewValue, &Options));
	}
}