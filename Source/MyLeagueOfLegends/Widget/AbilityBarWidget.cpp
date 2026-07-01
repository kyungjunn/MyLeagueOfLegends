// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityBarWidget.h"
#include "ActorComponents/SkillComponent.h"
#include "Widget/AbilitySlotWidget.h"

void UAbilityBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAbilityBarWidget::InitializeAbilityBar(USkillComponent* PlayerSkillComponent)
{
	if (!PlayerSkillComponent)
	{
		return;
	}

	if (AbilitySlot_Passive) AbilitySlot_Passive->InitSlot(PlayerSkillComponent, ESkillType::Passive);
	if (AbilitySlot_Q) AbilitySlot_Q->InitSlot(PlayerSkillComponent, ESkillType::Q);
	if (AbilitySlot_W) AbilitySlot_W->InitSlot(PlayerSkillComponent, ESkillType::W);
	if (AbilitySlot_E) AbilitySlot_E->InitSlot(PlayerSkillComponent, ESkillType::E);
	if (AbilitySlot_R) AbilitySlot_R->InitSlot(PlayerSkillComponent, ESkillType::R);
}
