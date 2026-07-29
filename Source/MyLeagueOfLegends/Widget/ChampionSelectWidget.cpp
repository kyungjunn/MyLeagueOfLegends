
#include "ChampionSelectWidget.h"
#include "Components/WrapBox.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/DataTable.h"
#include "ChampionSlotWidget.h"
#include "InGame/LOLPlayerState.h"
#include "ActorComponents/StatComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameInstance/LOLGameInstance.h"
#include "Network/AuthClient.h"

void UChampionSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ReadyButton)
    {
        ReadyButton->OnClicked.AddDynamic(this, &UChampionSelectWidget::OnReadyClicked);
    }

    // 위젯이 화면에 켜지자마자 리스트 자동 구성
    GenerateChampionList();

    // PlayerState의 변경 이벤트 구독
    //APlayerController* PC = GetOwningPlayer();
    //if (PC)
    //{
    //    if (ALOLPlayerState* MyPS = PC->GetPlayerState<ALOLPlayerState>())
    //    {
    //        MyPS->OnSelectionChanged.AddDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
    //    }
    //}
    //bSelfBound = false;
    //bEnemyBound = false;

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(BindingTimerHandle, this, &UChampionSelectWidget::TryBindPlayerStates, 0.2f, true);
    }

    RefreshSelectionUI();

    // 방 채팅 UI 연결
    if (ChatSendButton)
    {
        ChatSendButton->OnClicked.AddDynamic(this, &UChampionSelectWidget::OnChatSendClicked);
    }
    if (ChatInputBox)
    {
        ChatInputBox->OnTextCommitted.AddDynamic(this, &UChampionSelectWidget::OnChatInputCommitted);
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAuthClient* Auth = GI->GetSubsystem<UAuthClient>())
        {
            Auth->OnChatJoinRoomResult.AddDynamic(this, &UChampionSelectWidget::HandleChatJoinRoomResult);
            Auth->OnChatMessageReceived.AddDynamic(this, &UChampionSelectWidget::HandleChatMessageReceived);
            Auth->OnChatSystemMessage.AddDynamic(this, &UChampionSelectWidget::HandleChatSystemMessage);

            if (ULOLGameInstance* LOLGI = Cast<ULOLGameInstance>(GI))
            {
                FString RoomName;
                if (LOLGI->GetCurrentRoomName(RoomName))
                {
                    CachedRoomName = RoomName;
                    Auth->JoinChatRoom(CachedRoomName);
                }
            }
        }
    }
}

void UChampionSelectWidget::NativeDestruct()
{
    // 방 채팅 델리게이트 언바인딩 + 방 나가기
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAuthClient* Auth = GI->GetSubsystem<UAuthClient>())
        {
            Auth->OnChatJoinRoomResult.RemoveDynamic(this, &UChampionSelectWidget::HandleChatJoinRoomResult);
            Auth->OnChatMessageReceived.RemoveDynamic(this, &UChampionSelectWidget::HandleChatMessageReceived);
            Auth->OnChatSystemMessage.RemoveDynamic(this, &UChampionSelectWidget::HandleChatSystemMessage);

            if (!CachedRoomName.IsEmpty())
            {
                Auth->LeaveChatRoom(CachedRoomName);
            }
        }
    }

    // 메모리 누수 방지를 위한 언바인딩
    /*if (APlayerController* PC = GetOwningPlayer())
    {
        if (ALOLPlayerState* MyPS = PC->GetPlayerState<ALOLPlayerState>())
        {
            MyPS->OnSelectionChanged.RemoveDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
        }
    }*/
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(BindingTimerHandle);
    }

    // GameState를 순회하며 걸려있는 모든 델리게이트 안전하게 해제
    if (World && World->GetGameState())
    {
        for (APlayerState* BasePS : World->GetGameState()->PlayerArray)
        {
            if (ALOLPlayerState* TargetPS = Cast<ALOLPlayerState>(BasePS))
            {
                TargetPS->OnSelectionChanged.RemoveDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
            }
        }
    }
    Super::NativeDestruct();
}

void UChampionSelectWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    //APlayerController* LocalPC = GetOwningPlayer();

    // 자기 자신 바인딩 폴링
    //if (!bSelfBound && LocalPC)
    //{
    //    if (ALOLPlayerState* MyPS = LocalPC->GetPlayerState<ALOLPlayerState>())
    //    {
    //        MyPS->OnSelectionChanged.AddDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
    //        bSelfBound = true;
    //        UE_LOG(LogTemp, Warning, TEXT("[SelfBind] 성공"));
    //        RefreshSelectionUI();
    //    }
    //}

    // 상대방 바인딩 폴링
    //if (!bEnemyBound)
    //{
    //    UWorld* World = GetWorld();
    //    AGameStateBase* GS = World ? World->GetGameState() : nullptr;
    //    if (GS && LocalPC)
    //    {
    //        for (APlayerState* BasePS : GS->PlayerArray)
    //        {
    //            // 드디어 네트워크 선을 타고 상대방 PlayerState 패킷이 내 컴퓨터 월드에 안착했다면!
    //            if (BasePS && BasePS != LocalPC->PlayerState)
    //            {
    //                if (ALOLPlayerState* EnemyPS = Cast<ALOLPlayerState>(BasePS))
    //                {
    //                    // 안전하게 동적 바인딩 성공 시키고 루프 탈출
    //                    EnemyPS->OnSelectionChanged.AddDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
    //                    bEnemyBound = true;

    //                    // 바인딩 성공했으니 UI 즉시 한 번 새로고침
    //                    RefreshSelectionUI();
    //                    break;
    //                }
    //            }
    //        }
    //    }
    //}
}

void UChampionSelectWidget::TryBindPlayerStates()
{
    UWorld* World = GetWorld();
    AGameStateBase* GS = World ? World->GetGameState() : nullptr;
    if (!GS) return;

    bool bAllBound = true;
    APlayerController* LocalPC = GetOwningPlayer();
    if (!LocalPC) return;

    // 현재 방에 있는 인원 수만큼 동적 델리게이트를 걸어줍니다.
    for (APlayerState* BasePS : GS->PlayerArray)
    {
        if (ALOLPlayerState* TargetPS = Cast<ALOLPlayerState>(BasePS))
        {
            // 이미 등록되어 있다면 중복 등록 안 됨 (Ensure 에러 방지)
            if (!TargetPS->OnSelectionChanged.IsAlreadyBound(this, &UChampionSelectWidget::RefreshSelectionUI))
            {
                TargetPS->OnSelectionChanged.AddDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
                UE_LOG(LogTemp, Log, TEXT("[%s] 델리게이트 바인딩 성공!"), *TargetPS->GetPlayerName());
            }
        }
        else
        {
            // 아직 네트워킹 지연 때문에 PlayerState가 완전히 로딩 안 되었다면 다음 타이머 주기에 재시도
            bAllBound = false;
        }
    }

    // 모두 안전하게 바인딩 완료되었다면 타이머 해제
    if (bAllBound && GS->PlayerArray.Num() >= 2) // 최소 2명 이상 잡혔을 때 타이머 종료 규칙
    {
        World->GetTimerManager().ClearTimer(BindingTimerHandle);
        RefreshSelectionUI();
    }
}

void UChampionSelectWidget::GenerateChampionList()
{
    if (!ChampionDataTable || !ChampionSlotClass || !ChampionListBox) return;

    // 기존 리스트 청소
    ChampionListBox->ClearChildren();

    // 데이터 테이블의 모든 Row Name 정보 확보
    TArray<FName> RowNames = ChampionDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FChampionStatRow* StatRow = ChampionDataTable->FindRow<FChampionStatRow>(RowName, TEXT(""));
        if (StatRow)
        {
            // 슬롯 위젯 동적 생성
            UChampionSlotWidget* NewSlot = CreateWidget<UChampionSlotWidget>(GetOwningPlayer(), ChampionSlotClass);
            if (NewSlot)
            {
                NewSlot->InitializeSlot(RowName, StatRow->CharacterIcon);

                //  WrapBox 자식에 추가
                ChampionListBox->AddChildToWrapBox(NewSlot);
            }
        }
    }
}

void UChampionSelectWidget::RefreshSelectionUI()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AGameStateBase* GS = World->GetGameState();
    if (!GS) return;

    APlayerController* LocalPC = GetOwningPlayer();
    if (!LocalPC) return;

    ALOLPlayerState* MyPS = LocalPC->GetPlayerState<ALOLPlayerState>();
    if (!MyPS) return; // 튕김 방지 안전장치: 내 State가 안 넘어왔으면 중단

    FString NetRoleStr = (LocalPC->GetNetMode() == NM_Client) ? TEXT("Client") : TEXT("Server(Host)");
    FString MyTeamStr = (MyPS->GetTeam() == ETeam::Blue) ? TEXT("Blue") : TEXT("Red");
    UE_LOG(LogTemp, Warning, TEXT("=== [UI Refresh Started] 화면 주인: %s | 내 팀: %s ==="), *NetRoleStr, *MyTeamStr);

    // 기본 상태 초기화
    if (BluePlayerNameText) BluePlayerNameText->SetText(FText::FromString(TEXT("선택 중...")));
    if (RedPlayerNameText) RedPlayerNameText->SetText(FText::FromString(TEXT("선택 중...")));
    if (BlueChampionImage) BlueChampionImage->SetOpacity(1.0f); // 혹은 디폴트 텍스처
    if (RedChampionImage) RedChampionImage->SetOpacity(1.0f);

    // 현재 월드에 있는 모든 유저의 PlayerState를 조사
    for (APlayerState* BasePS : GS->PlayerArray)
    {
        ALOLPlayerState* TargetPS = Cast<ALOLPlayerState>(BasePS);
        if (!TargetPS) continue;

        if (TargetPS != MyPS)
        {
            //UE_LOG(LogTemp, Verbose, TEXT("    => %s 유저는 내가 아니므로 UI 렌더링 스킵합니다."), *TargetPS->GetPlayerName());
            continue;
        }

        //FString TargetTeamStr = (TargetPS->GetTeam() == ETeam::Blue) ? TEXT("Blue") : TEXT("Red");
        //UE_LOG(LogTemp, Log, TEXT(" -> 루프 탐색 중인 유저: %s | 팀: %s | 고른 챔피언: %s"),
        //    *TargetPS->GetPlayerName(), *TargetTeamStr, *TargetPS->GetSelectedChampion().ToString());


        FName SelectedRow = TargetPS->GetSelectedChampion();
        FString TargetName = TargetPS->GetPlayerName();

        UTexture2D* TargetIcon = nullptr;
        // 고른 챔피언이 있다면 데이터 테이블에서 아이콘 검색
        if (!SelectedRow.IsNone() && ChampionDataTable)
        {
            FChampionStatRow* StatRow = ChampionDataTable->FindRow<FChampionStatRow>(SelectedRow, TEXT(""));
            if (StatRow)
            {
                TargetIcon = StatRow->CharacterIcon;
            }
        }

        if (!TargetIcon)
        {
            //UE_LOG(LogTemp, Error, TEXT("    => [실패] 내가 고른 %s 챔피언의 아이콘을 데이터테이블에서 찾지 못했습니다!"), *SelectedRow.ToString());
            continue;
        }

        // 피아 식별 후 알맞은 픽칸 이미지 컴포넌트에 세팅
        if (TargetPS->GetTeam() == ETeam::Blue)
        {
            if (BluePlayerNameText)
            {
                BluePlayerNameText->SetText(FText::FromString(TargetName));
            }
            if (BlueChampionImage)
            {
                //UE_LOG(LogTemp, Warning, TEXT("    => [렌더링] 내가 Blue팀이므로 BlueChampionImage를 %s 아이콘으로 바꿉니다!"), *SelectedRow.ToString());
                BlueChampionImage->SetColorAndOpacity(FLinearColor::White);
                BlueChampionImage->SetBrushFromTexture(TargetIcon);
                BlueChampionImage->SetOpacity(1.0f);
            }
        }
        else if (TargetPS->GetTeam() == ETeam::Red)
        {
            if (RedPlayerNameText)
            {
                RedPlayerNameText->SetText(FText::FromString(TargetName));
            }
            if (RedChampionImage)
            {
                //UE_LOG(LogTemp, Warning, TEXT("    => [렌더링] 내가 Red팀이므로 RedChampionImage를 %s 아이콘으로 바꿉니다!"), *SelectedRow.ToString());
                RedChampionImage->SetColorAndOpacity(FLinearColor::White);
                RedChampionImage->SetColorAndOpacity(FLinearColor::White);
                RedChampionImage->SetBrushFromTexture(TargetIcon);
                RedChampionImage->SetOpacity(1.0f);
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("=== [UI Refresh Ended] ==="));
}

void UChampionSelectWidget::OnReadyClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ALOLPlayerState* PS = PC->GetPlayerState<ALOLPlayerState>())
        {
            if (PS->GetSelectedChampion().IsNone()) return;

            // 서버에 준비 완료
            PS->Server_ReadyUp();

            // 중복 클릭 밴
            ReadyButton->SetIsEnabled(false);
        }
    }
}

void UChampionSelectWidget::HandleChatJoinRoomResult(bool bSuccess, const FString& RoomName, const FString& ErrorMessage)
{
    if (RoomName != CachedRoomName) return;

    if (!bSuccess)
    {
        AppendChatLine(FString::Printf(TEXT("[System] failed to join chat room: %s"), *ErrorMessage));
    }
}

void UChampionSelectWidget::HandleChatMessageReceived(const FString& RoomName, const FString& Sender, const FString& Message, const FString& Timestamp)
{
    if (RoomName != CachedRoomName) return;

    AppendChatLine(FString::Printf(TEXT("%s: %s"), *Sender, *Message));
}

void UChampionSelectWidget::HandleChatSystemMessage(const FString& RoomName, const FString& Message)
{
    if (RoomName != CachedRoomName) return;

    AppendChatLine(FString::Printf(TEXT("[System] %s"), *Message));
}

void UChampionSelectWidget::OnChatSendClicked()
{
    if (!ChatInputBox || CachedRoomName.IsEmpty()) return;

    const FString Text = ChatInputBox->GetText().ToString();
    if (Text.IsEmpty()) return;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAuthClient* Auth = GI->GetSubsystem<UAuthClient>())
        {
            Auth->SendChatMessage(CachedRoomName, Text);
        }
    }

    ChatInputBox->SetText(FText::GetEmpty());
}

void UChampionSelectWidget::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        OnChatSendClicked();
    }
}

void UChampionSelectWidget::AppendChatLine(const FString& Line)
{
    if (!ChatScrollBox || !WidgetTree) return;

    // UTextBlock is a UWidget, not a UUserWidget -- CreateWidget<>() only
    // constructs UUserWidget instances, so a plain leaf widget like this has
    // to go through this UserWidget's own WidgetTree instead.
    UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (!TextBlock) return;

    TextBlock->SetText(FText::FromString(Line));
    ChatScrollBox->AddChild(TextBlock);
    ChatScrollBox->ScrollToEnd();
}