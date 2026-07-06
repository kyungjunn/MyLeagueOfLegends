// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidget.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
class USignupWidget;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ---- 로그인 관련 컴포넌트 (WBP_Login 구조와 매칭) ----
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> LoginIdTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> LoginPasswordTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoginButton;

	// "지금 가입하세요!" 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GoToSignupButton;

	// 하단 상태 및 로그인 메시지용 텍스트 블록
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	// ---- 설정 및 클래스 연결 ----
	// 디테일 패널에서 WBP_Signup을 선택하도록 설정
	UPROPERTY(EditDefaultsOnly, Category = "UI Setup")
	TSubclassOf<USignupWidget> SignupWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI Setup")
	FName MainMenuLevelName = FName(TEXT("MainMenu"));

private:
	UFUNCTION()
	void OnLoginClicked();

	UFUNCTION()
	void OnGoToSignupClicked();

	UFUNCTION()
	void HandleLoginResult(bool bSuccess, const FString& Nickname, const FString& Message);
};
