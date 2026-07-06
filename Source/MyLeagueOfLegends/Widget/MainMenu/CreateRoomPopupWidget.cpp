// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateRoomPopupWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "GameInstance/LOLGameInstance.h"

void UCreateRoomPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmCreateButton) ConfirmCreateButton->OnClicked.AddDynamic(this, &UCreateRoomPopupWidget::OnConfirmCreateClicked);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &UCreateRoomPopupWidget::OnCloseClicked);
}

void UCreateRoomPopupWidget::OnConfirmCreateClicked()
{
	if (!RoomNameTextBox) return;

	FString RoomName = RoomNameTextBox->GetText().ToString();
	if (RoomName.IsEmpty()) return;

	if (ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>())
	{
		// 기존에 GI에 구현되어 있던 CreateRoom을 호출합니다. (최대 인원 2명 기본값)
		GI->CreateRoom(RoomName, 2);
	}
}

void UCreateRoomPopupWidget::OnCloseClicked()
{
	RemoveFromParent();
}