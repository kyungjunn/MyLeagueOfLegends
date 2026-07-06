#include "LoginWidget.h"
#include "Widget/Login/SignupWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameInstance/LOLGameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"

#include "Kismet/GameplayStatics.h"
#include "Network/AuthClient.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoginButton) LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::OnLoginClicked);
	if (GoToSignupButton) GoToSignupButton->OnClicked.AddDynamic(this, &ULoginWidget::OnGoToSignupClicked);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAuthClient* AuthSubsystem = GI->GetSubsystem<UAuthClient>())
		{
			AuthSubsystem->OnLoginResult.AddDynamic(this, &ULoginWidget::HandleLoginResult);
		}
	}
}

void ULoginWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAuthClient* AuthSubsystem = GI->GetSubsystem<UAuthClient>())
		{
			AuthSubsystem->OnLoginResult.RemoveDynamic(this, &ULoginWidget::HandleLoginResult);
		}
	}

	Super::NativeDestruct();
}

void ULoginWidget::OnLoginClicked()
{
	if (!LoginIdTextBox || !LoginPasswordTextBox) return;

	const FString Id = LoginIdTextBox->GetText().ToString();
	const FString Pw = LoginPasswordTextBox->GetText().ToString();

	if (Id.IsEmpty() || Pw.IsEmpty())
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Input ID, password")));
		return;
	}

	if (StatusText) StatusText->SetText(FText::FromString(TEXT("Login ... ing ..")));
	if (LoginButton) LoginButton->SetIsEnabled(false);

	if (ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>())
	{
		if (UAuthClient* AuthSubsystem = GI->GetSubsystem<UAuthClient>())
		{
			AuthSubsystem->Login(Id, Pw);
		}
	}
}

void ULoginWidget::OnGoToSignupClicked()
{
	// 에디터 디테일 패널에서 SignupWidgetClass가 올바르게 할당되었는지 확인합니다.
	if (!SignupWidgetClass)
	{
		return;
	}

	// 로그인 화면 위에 회원가입 위젯 생성 후 띄우기
	USignupWidget* SignupWidgetInstance = CreateWidget<USignupWidget>(this, SignupWidgetClass);
	if (SignupWidgetInstance)
	{
		SignupWidgetInstance->AddToViewport();
	}
}

void ULoginWidget::HandleLoginResult(bool bSuccess, const FString& Nickname, const FString& Message)
{
	if (LoginButton) LoginButton->SetIsEnabled(true);
	if (StatusText) StatusText->SetText(FText::FromString(Message));

	if (bSuccess)
	{
		// 로그인 성공 시, 게임 인스턴스에 유저 정보를 캐싱해 둡니다.
		if (ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>())
		{
			GI->UserId = LoginIdTextBox->GetText().ToString();
			GI->UserNickname = Nickname;
		}
		UGameplayStatics::OpenLevel(this, MainMenuLevelName, true);
	}
}