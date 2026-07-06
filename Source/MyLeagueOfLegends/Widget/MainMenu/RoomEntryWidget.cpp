// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void URoomEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (RoomListButton) RoomListButton->OnClicked.AddDynamic(this, &URoomEntryWidget::OnJoinClicked);
}

void URoomEntryWidget::SetupEntry(const FRoomInfo& InRoomInfo)
{
	TargetSearchIndex = InRoomInfo.SearchResultIndex;

	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(InRoomInfo.RoomName));
	}

	if (PlayerCountText)
	{
		FString CountStr = FString::Printf(TEXT("%d / %d"), InRoomInfo.CurrentPlayers, InRoomInfo.MaxPlayers);
		PlayerCountText->SetText(FText::FromString(CountStr));
	}
}

void URoomEntryWidget::OnJoinClicked()
{
	if (TargetSearchIndex == -1) return;

	if (ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>())
	{
		// GameInstance의 세션 인덱스 참가 함수 호출
		GI->JoinRoomByIndex(TargetSearchIndex);
	}
}