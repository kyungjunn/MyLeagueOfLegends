// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySlotWidget.h"
#include "ActorComponents/SkillComponent.h"
#include "DataAssets/SkillDataAsset.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"


void UAbilitySlotWidget::InitSlot(USkillComponent* InSkillComponent, ESkillType InSkillType)
{
	if (InSkillComponent == nullptr)
	{
		return;
	}

	TargetSkillComponent = InSkillComponent;
	AssignedSkillType = InSkillType;

	// 스킬컴포넌트의 슬롯에서 데이터 에셋 캐싱
	if (TargetSkillComponent->SkillSlots.Contains(AssignedSkillType))
	{
		CachedSkillData = TargetSkillComponent->SkillSlots[AssignedSkillType];
	}

	// 스킬 아이콘 
	if (CachedSkillData && CachedSkillData->SkillIcon && Icon_Image)
	{
		Icon_Image->SetBrushFromTexture(CachedSkillData->SkillIcon);
	}

	// 처음엔 쿨타임 텍스트 숨김
	if (CoolTime_Text)
	{
		CoolTime_Text->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Cooldown_Image)
	{
		Cooldown_Image->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAbilitySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Cooldown_Image)
	{
		CooldownMaterialDynamic = Cooldown_Image->GetDynamicMaterial();
	}
}

void UAbilitySlotWidget::NativeTick(const FGeometry& MyGeomerty, float InDeltaTime)
{
	Super::NativeTick(MyGeomerty, InDeltaTime);

	UpdateSlot();
}

void UAbilitySlotWidget::UpdateSlot()
{
	if (!TargetSkillComponent || !CachedSkillData)
	{
		return;
	}

	bool bOnCooldown = TargetSkillComponent->IsSkillOnCooldown(AssignedSkillType);

	if (bOnCooldown)
	{
		float RemainingTime = TargetSkillComponent->GetSkillRemainingCooldown(AssignedSkillType);
		float MaxCooldown = CachedSkillData->CoolDown;

		// 쿨타임 표시
		if (CoolTime_Text)
		{
			CoolTime_Text->SetVisibility(ESlateVisibility::HitTestInvisible);

			// 1초 이상이면 정수, 1초 미만이면 소수 첫째자리까지
			FText CoolText;
			if (RemainingTime >= 1.0f)
			{
				CoolText = FText::AsNumber(FMath::CeilToInt(RemainingTime));
			}
			else
			{
				FNumberFormattingOptions Options;
				Options.MaximumFractionalDigits = 1;
				Options.MinimumFractionalDigits = 1;
				CoolText = FText::AsNumber(RemainingTime, &Options);
			}

			CoolTime_Text->SetText(CoolText);
		}

		// 쿨타임 이미지 Progress
		if (Cooldown_Image)
		{
			Cooldown_Image->SetVisibility(ESlateVisibility::HitTestInvisible);

			if (CooldownMaterialDynamic && MaxCooldown > 0.f)
			{
				// 쿨타임 비율 계산
				// RemainingTime / MaxCooldown 
				float ProgressValue = RemainingTime / MaxCooldown;

				// 머티리얼의 Progress 파라미터 값 변경
				CooldownMaterialDynamic->SetScalarParameterValue(TEXT("Progress"), ProgressValue);
			}
		}
	}
	else // 쿨타임이 아니라면
	{
		if (CoolTime_Text && CoolTime_Text->GetVisibility() != ESlateVisibility::Hidden)
		{
			CoolTime_Text->SetVisibility(ESlateVisibility::Hidden);
		}

		if (Cooldown_Image && Cooldown_Image->GetVisibility() != ESlateVisibility::Hidden)
		{
			Cooldown_Image->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
