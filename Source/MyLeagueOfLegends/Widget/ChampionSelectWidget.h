// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "ChampionSelectWidget.generated.h"

class UWrapBox;
class UButton;
class UImage;
class UTextBlock;
class UChampionSlotWidget;
class UDataTable;
class UScrollBox;
class UEditableTextBox;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API UChampionSelectWidget : public UUserWidget
{
	GENERATED_BODY()

	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> ChampionListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BlueChampionImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RedChampionImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BluePlayerNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RedPlayerNameText;

	// Room chat UI. Optional so this widget still compiles/works before the
	// Blueprint (WBP_ChampionSelect or similar) has these widgets added in
	// the UMG designer with matching names.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> ChatScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> ChatInputBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ChatSendButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UDataTable> ChampionDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TSubclassOf<UChampionSlotWidget> ChampionSlotClass;

public:
	// PlayerState���� ���� �˸��� �޾� UI�� ���������� �ݹ� �Լ�
	UFUNCTION()
	void RefreshSelectionUI();

private:
	UFUNCTION()
	void OnReadyClicked();

	void GenerateChampionList();

	bool bEnemyBound = false;
	bool bSelfBound = false;

	// �ֱ������� PlayerState ������ ������ Ÿ�̸� �ڵ�
	FTimerHandle BindingTimerHandle;

	// Ÿ�̸Ӱ� ȣ���� ���ε� �Լ�
	void TryBindPlayerStates();

	// Room chat: joined once at NativeConstruct using the current OnlineSubsystem
	// session's RoomName (see ULOLGameInstance::GetCurrentRoomName), left again at
	// NativeDestruct.
	FString CachedRoomName;

	UFUNCTION()
	void HandleChatJoinRoomResult(bool bSuccess, const FString& RoomName, const FString& ErrorMessage);

	UFUNCTION()
	void HandleChatMessageReceived(const FString& RoomName, const FString& Sender, const FString& Message, const FString& Timestamp);

	UFUNCTION()
	void HandleChatSystemMessage(const FString& RoomName, const FString& Message);

	UFUNCTION()
	void OnChatSendClicked();

	UFUNCTION()
	void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void AppendChatLine(const FString& Line);
};
