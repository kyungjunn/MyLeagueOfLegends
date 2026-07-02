
#include "ChampionSelectWidget.h"
#include "Components/WrapBox.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"
#include "ChampionSlotWidget.h"
#include "InGame/LOLPlayerState.h"
#include "ActorComponents/StatComponent.h"
#include "GameFramework/GameStateBase.h"

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
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        if (ALOLPlayerState* MyPS = PC->GetPlayerState<ALOLPlayerState>())
        {
            MyPS->OnSelectionChanged.AddDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
        }
    }

    // 상대방 정보 실시간 동기화 바인딩
    //if (UWorld* World = GetWorld())
    //{
    //    if (AGameStateBase* GS = World->GetGameState())
    //    {
    //        for (APlayerState* BasePS : GS->PlayerArray)
    //        {
    //            // 상대방 PlayerState를 찾아서 OnSelectionChanged 이벤트 연결
    //            if (BasePS && BasePS != PC->PlayerState)
    //            {
    //                if (ALOLPlayerState* EnemyPS = Cast<ALOLPlayerState>(BasePS))
    //                {
    //                    EnemyPS->OnSelectionChanged.AddDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
    //                }
    //            }
    //        }
    //    }
    //}

    bEnemyBound = false;
    RefreshSelectionUI();
}

void UChampionSelectWidget::NativeDestruct()
{
    // 메모리 누수 방지를 위한 언바인딩
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ALOLPlayerState* MyPS = PC->GetPlayerState<ALOLPlayerState>())
        {
            MyPS->OnSelectionChanged.RemoveDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
        }
    }
    Super::NativeDestruct();
}

void UChampionSelectWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 상대방 바인딩이 아직 안 끝났다면 끈질기게 매 프레임 GameState를 안전 검사합니다.
    if (!bEnemyBound)
    {
        UWorld* World = GetWorld();
        AGameStateBase* GS = World ? World->GetGameState() : nullptr;
        APlayerController* LocalPC = GetOwningPlayer();

        if (GS && LocalPC)
        {
            for (APlayerState* BasePS : GS->PlayerArray)
            {
                // 드디어 네트워크 선을 타고 상대방 PlayerState 패킷이 내 컴퓨터 월드에 안착했다면!
                if (BasePS && BasePS != LocalPC->PlayerState)
                {
                    if (ALOLPlayerState* EnemyPS = Cast<ALOLPlayerState>(BasePS))
                    {
                        // 안전하게 동적 바인딩 성공 시키고 루프 탈출
                        EnemyPS->OnSelectionChanged.AddDynamic(this, &UChampionSelectWidget::RefreshSelectionUI);
                        bEnemyBound = true;

                        // 바인딩 성공했으니 UI 즉시 한 번 새로고침
                        RefreshSelectionUI();
                        break;
                    }
                }
            }
        }
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

    // 현재 월드에 있는 모든 유저의 PlayerState를 조사
    for (APlayerState* BasePS : GS->PlayerArray)
    {
        ALOLPlayerState* TargetPS = Cast<ALOLPlayerState>(BasePS);
        if (!TargetPS) continue;

        FName SelectedRow = TargetPS->GetSelectedChampion();
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

        // 피아 식별 후 알맞은 픽칸 이미지 컴포넌트에 세팅
        if (TargetPS == MyPS)
        {
            if (BlueChampionImage && TargetIcon)
            {
                BlueChampionImage->SetColorAndOpacity(FLinearColor::White);
                BlueChampionImage->SetBrushFromTexture(TargetIcon);
                BlueChampionImage->SetOpacity(1.0f);
            }
        }
        else // 상대방 유저인 경우
        {
            if (RedChampionImage && TargetIcon)
            {
                BlueChampionImage->SetColorAndOpacity(FLinearColor::White);
                RedChampionImage->SetBrushFromTexture(TargetIcon);
                RedChampionImage->SetOpacity(1.0f);
            }
        }
    }
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