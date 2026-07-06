// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Widget/MainMenu/CreateRoomPopupWidget.h"
#include "Widget/MainMenu/RoomEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "GameInstance/LOLGameInstance.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartGameButton) StartGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);

	ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>();
	if (!GI) return;

	// 1. 앞 전 로그인 단계에서 캐싱해 둔 유저 닉네임을 UI 상단에 적용
	if (NicknameText)
	{
		NicknameText->SetText(FText::FromString(GI->UserNickname));
	}

	// 2. 방 리스트가 서버/세션 서브시스템으로부터 업데이트되었을 때 갱신 델리게이트 등록
	GI->OnRoomListUpdated.AddDynamic(this, &UMainMenuWidget::RefreshRoomList);

	//진입하자마자 1회 검색 후, 3초마다 자동으로 TriggerFindRooms를 반복 호출합니다.
	TriggerFindRooms();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RoomRefreshTimerHandle, this, &UMainMenuWidget::TriggerFindRooms, 3.0f, true);
	}
}

void UMainMenuWidget::NativeDestruct()
{
	// 위젯이 닫힐 때 타이머를 안전하게 꺼줍니다 (메모리 가비지 방지)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoomRefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UMainMenuWidget::OnStartGameClicked()
{
	if (!CreateRoomPopupClass) return;

	// 회원가입 창 방식처럼 메인화면 위에 방 생성 팝업 위젯 얹기
	UCreateRoomPopupWidget* PopupInstance = CreateWidget<UCreateRoomPopupWidget>(this, CreateRoomPopupClass);
	if (PopupInstance)
	{
		PopupInstance->AddToViewport();
	}
}

void UMainMenuWidget::RefreshRoomList()
{
	if (!RoomListScrollBox || !RoomEntryWidgetClass) return;

	// 기존 목록 싹 비우기
	RoomListScrollBox->ClearChildren();

	ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>();
	if (!GI) return;

	// GameInstance가 들고 있는 세션 결과물 배열을 순회하며 위젯 동적 생성
	for (const FRoomInfo& RoomData : GI->RoomList)
	{
		URoomEntryWidget* EntryWidget = CreateWidget<URoomEntryWidget>(this, RoomEntryWidgetClass);
		if (EntryWidget)
		{
			EntryWidget->SetupEntry(RoomData);
			// 스크롤 박스 내부에 Vertical(세로) 형태로 한 칸씩 자동 누적 배치됨
			RoomListScrollBox->AddChild(EntryWidget);
		}
	}
}

void UMainMenuWidget::TriggerFindRooms()
{
	if (ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>())
	{
		// 지속적으로 세션 서브시스템에 최신 방 목록을 요청합니다.
		GI->FindRooms();
	}
}