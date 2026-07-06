// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "GameInstance/LOLGameInstance.h"
#include "Widget/ChampionSelectWidget.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 서버(리슨 서버 호스트) 포함, 화면을 실제로 가진 로컬 컨트롤러만 위젯 생성
	if (IsLocalController())
	{
		if (ULOLGameInstance* GI = GetGameInstance<ULOLGameInstance>())
		{
			if (!GI->UserNickname.IsEmpty())
			{
				Server_SetNickname(GI->UserNickname);
			}
		}

		if (ChampionSelectWidgetClass)
		{
			ChampionSelectWidgetInstance = CreateWidget<UChampionSelectWidget>(this, ChampionSelectWidgetClass);
			if (ChampionSelectWidgetInstance)
			{
				ChampionSelectWidgetInstance->AddToViewport();

				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(ChampionSelectWidgetInstance->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetInputMode(InputMode);

				bShowMouseCursor = true;
			}
		}
	}
}

void ALobbyPlayerController::Server_SetNickname_Implementation(const FString& Nickname)
{
	if (ALOLPlayerState* PS = GetPlayerState<ALOLPlayerState>())
	{
		PS->SetPlayerName(Nickname);
	}
}
