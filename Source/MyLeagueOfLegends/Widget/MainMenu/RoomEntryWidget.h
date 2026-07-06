// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameInstance/LOLGameInstance.h"
#include "RoomEntryWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API URoomEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 이 항목이 들고 있어야 할 방 정보 세팅 함수
	void SetupEntry(const FRoomInfo& InRoomInfo);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RoomNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText; // 예: "1 / 2" 형태로 표시

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RoomListButton;

private:
	UFUNCTION()
	void OnJoinClicked();

	// 세션 탐색 결과 배열의 몇 번째 인덱스인지 저장
	int32 TargetSearchIndex = -1;
};
