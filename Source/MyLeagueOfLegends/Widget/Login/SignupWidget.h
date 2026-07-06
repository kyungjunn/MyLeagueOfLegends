// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SignupWidget.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;

UCLASS()
class MYLEAGUEOFLEGENDS_API USignupWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// WBP_Signup의 컴포넌트 구조와 이름 맞춤
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> SignupIdTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> SignupPasswordTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> SignupNicknameTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SignupButton;

	// 우측 상단 X 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	// 하단 상태 메세지 (Success 혹은 오류 텍스트 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

private:
	UFUNCTION()
	void OnSignupClicked();

	UFUNCTION()
	void OnExitClicked();

	UFUNCTION()
	void HandleSignupResult(bool bSuccess, const FString& Message);
};
