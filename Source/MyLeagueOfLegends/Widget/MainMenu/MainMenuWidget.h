// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class UCreateRoomPopupWidget;
class URoomEntryWidget;
/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override; // 타이머 해제를 위해 추가

	// ---- 상단 및 배경 관련 바인딩 ----
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NicknameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton;

	// ---- 하단 방 목록 세로 리스트 바인딩 ----
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> RoomListScrollBox;

	// ---- 생성용 서브 블루프린트 클래스 세팅 ----
	UPROPERTY(EditDefaultsOnly, Category = "UI Setup")
	TSubclassOf<UCreateRoomPopupWidget> CreateRoomPopupClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI Setup")
	TSubclassOf<URoomEntryWidget> RoomEntryWidgetClass;

private:
	UFUNCTION()
	void OnStartGameClicked();

	// 게임 인스턴스 델리게이트를 통해 세션 검색이 끝나면 호출될 리스트 갱신 함수
	UFUNCTION()
	void RefreshRoomList();

	// 주기적 자동 검색을 위한 타이머 핸들
	FTimerHandle RoomRefreshTimerHandle;

	// 타이머에 의해 호출될 헬퍼 함수
	void TriggerFindRooms();
};
