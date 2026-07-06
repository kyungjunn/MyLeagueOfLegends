#include "SignupWidget.h"
#include "Network/AuthClient.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameInstance/LOLGameInstance.h"

void USignupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SignupButton) SignupButton->OnClicked.AddDynamic(this, &USignupWidget::OnSignupClicked);
	if (ExitButton) ExitButton->OnClicked.AddDynamic(this, &USignupWidget::OnExitClicked);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAuthClient* AuthSubsystem = GI->GetSubsystem<UAuthClient>())
		{
			AuthSubsystem->OnSignupResult.AddDynamic(this, &USignupWidget::HandleSignupResult);
		}
	}
}

void USignupWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAuthClient* AuthSubsystem = GI->GetSubsystem<UAuthClient>())
		{
			AuthSubsystem->OnSignupResult.AddDynamic(this, &USignupWidget::HandleSignupResult);
		}
	}

	Super::NativeDestruct();
}

void USignupWidget::OnSignupClicked()
{
	if (!SignupIdTextBox || !SignupPasswordTextBox || !SignupNicknameTextBox) return;

	const FString Id = SignupIdTextBox->GetText().ToString();
	const FString Pw = SignupPasswordTextBox->GetText().ToString();
	const FString Nickname = SignupNicknameTextBox->GetText().ToString();

	if (Id.IsEmpty() || Pw.IsEmpty() || Nickname.IsEmpty())
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Input id pw nickname")));
		return;
	}

	if (StatusText) StatusText->SetText(FText::FromString(TEXT("Signup ...")));
	SignupButton->SetIsEnabled(false);

	if (ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>())
	{
		if (UAuthClient* AuthSubsystem = GI->GetSubsystem<UAuthClient>())
		{
			AuthSubsystem->Signup(Id, Pw, Nickname);
		}
	}
}

void USignupWidget::OnExitClicked()
{
	// 화면에서 제거
	RemoveFromParent();
}

void USignupWidget::HandleSignupResult(bool bSuccess, const FString& Message)
{
	if (SignupButton) SignupButton->SetIsEnabled(true);
	if (StatusText) StatusText->SetText(FText::FromString(Message));

	if (bSuccess)
	{
		// 회원가입 성공 시 잠시 후 닫히거나 즉시 회원가입 창을 닫음
		RemoveFromParent();
	}
}