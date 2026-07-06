// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateRoomPopupWidget.generated.h"

class UEditableTextBox;
class UButton;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UCreateRoomPopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> RoomNameTextBox;

	// 실제 방을 생성하는 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmCreateButton;

	// 팝업을 닫는 X 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

private:
	UFUNCTION()
	void OnConfirmCreateClicked();

	UFUNCTION()
	void OnCloseClicked();
};
