// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "LOLGameMode.h"
#include "Lobby/LobbyGameMode.h"
#include "GameInstance/LOLGameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "EngineUtils.h"

// [진단용] 원인 확정 후 제거할 것
static const TCHAR* LOLNetModeToString(const UWorld* World)
{
	if (!World) { return TEXT("NoWorld"); }
	switch (World->GetNetMode())
	{
	case NM_Standalone:      return TEXT("Standalone");
	case NM_ListenServer:    return TEXT("ListenServer");
	case NM_DedicatedServer: return TEXT("DedicatedServer");
	case NM_Client:          return TEXT("Client");
	default:                 return TEXT("?");
	}
}

ALOLPlayerState::ALOLPlayerState()
{
	Team = ETeam::None;
	SelectedChampion = NAME_None;
	bIsReady = false;
	bAlwaysRelevant = true; // 멀티 UI 동기화 안정성

	// APlayerState 기본값은 SetNetUpdateFrequency(1) = 초당 1회다.
	// 이름/점수/핑처럼 거의 안 변하는 데이터 기준이라, 골드를 얹으면 클라에 최대 1초 늦게 도착한다.
	// (인벤토리 Slots 는 Pawn 에 붙어 100Hz 로 도니 아이템만 즉시 뜨고 골드만 늦는 비대칭이 생긴다)
	SetNetUpdateFrequency(10.f);

	// 여기서 InitialGold 를 넣으면 안 된다. 생성자는 CDO 에도 돌기 때문에 CDO 의 Gold 까지 같은 값이 되고,
	// UE 초기 복제는 "클래스 기본값과 다른 프로퍼티"만 최초 번치에 실으므로 Gold 가 통째로 생략된다.
	// (= 접속한 클라이언트에서 OnRep_Gold 가 한 번도 안 뜬다)
	// 게다가 BP 디폴트로 InitialGold 를 바꿔도 생성자가 이미 지나간 뒤라 반영되지 않는다.
	// 실제 지급은 서버 PostInitializeComponents 에서 한다.
	Gold = 0;
}

void ALOLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALOLPlayerState, Team);
	DOREPLIFETIME(ALOLPlayerState, SelectedChampion);
	DOREPLIFETIME(ALOLPlayerState, bIsReady);
	DOREPLIFETIME(ALOLPlayerState, Gold);
}

// 시작 골드는 서버에서 PostInitializeComponents 에 지급한다. BeginPlay 보다 이르러야
// 위젯이 먼저 Gold 를 읽어 0 을 보는 일이 없다. CDO 기본값(0)과 달라지므로 클라이언트로 정상 복제된다.
void ALOLPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// [진단용] PlayerState 액터가 각 사이드에 언제 생기는지 추적. 원인 확정 후 제거할 것
	UE_LOG(LogTemp, Warning, TEXT("[%s] LOLPlayerState::PostInitComp T=%.2f | Name=%s Class=%s HasAuth=%d Owner=%s"),
		LOLNetModeToString(GetWorld()), GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f,
		*GetName(), *GetClass()->GetName(), HasAuthority() ? 1 : 0, *GetNameSafe(GetOwner()));

	if (HasAuthority())
	{
		Gold = InitialGold;
		OnRep_Gold(); // 리슨서버 호스트는 OnRep 이 안 오므로 직접 호출
	}
}

void ALOLPlayerState::C2S_SelectChampion_Implementation(FName ChampionRowName)
{
	if (bIsReady) // 레디 박았으면 변경 불가
	{
		return;
	}

	SelectedChampion = ChampionRowName;
	OnRep_SelectedChampion();
}

void ALOLPlayerState::Server_ReadyUp_Implementation()
{
	if (SelectedChampion.IsNone()) // 캐릭터 안고르면 레디 불가
	{
		return;
	}
	bIsReady = true;

	if (ALobbyGameMode* LobbyGM = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		LobbyGM->CheckAllPlayersReady();
	}
}

void ALOLPlayerState::OnRep_Team()
{
	OnSelectionChanged.Broadcast();
	SaveToLocalGameInstance();
}

void ALOLPlayerState::OnRep_SelectedChampion()
{
	OnSelectionChanged.Broadcast();
	SaveToLocalGameInstance();
}

void ALOLPlayerState::SaveToLocalGameInstance()
{
	// 이 PlayerState가 "내 컴퓨터의 내 캐릭터" 것인지 확인
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || LocalPC->PlayerState != this)
	{
		return; // 상대방 PlayerState면 무시
	}

	if (ULOLGameInstance* GI = GetWorld()->GetGameInstance<ULOLGameInstance>())
	{
		GI->SavedTeam = Team;
		GI->SavedSelectedChampion = SelectedChampion;
	}
}

void ALOLPlayerState::OnRep_Gold()
{
	OnGoldChanged.Broadcast(Gold);
}

// 서버 전용: 골드 차감. 부족하거나 권한 없으면 false
bool ALOLPlayerState::SpendGold(int32 Amount)
{
	if (!HasAuthority() || Amount < 0 || Gold < Amount)
	{
		return false;
	}
	Gold -= Amount;
	OnRep_Gold();     // 리슨서버 호스트는 OnRep이 안 오므로 직접 호출
	ForceNetUpdate(); // 다음 정기 복제까지 기다리지 않고 즉시 클라로 밀어준다
	return true;
}

// 서버 전용: 골드 획득/환급
void ALOLPlayerState::AddGold(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}
	Gold += Amount;
	OnRep_Gold();
	ForceNetUpdate();
}

// 로컬 플레이어 자신의 PlayerState. 서버/클라이언트 모두에서 정확히 "자기 것"을 돌려준다.
ALOLPlayerState* ALOLPlayerState::GetLocalLOLPlayerState(const UObject* WorldContextObject)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	const TCHAR* NetModeStr = LOLNetModeToString(World);

	// 클라이언트에는 로컬 컨트롤러가 하나뿐이고, 리슨서버 호스트에서는 호스트 자신이 잡힌다.
	int32 PCCount = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}
		++PCCount;

		if (!PC->IsLocalController())
		{
			continue;
		}

		if (!PC->PlayerState)
		{
			// 클라 월드에 PlayerState 액터가 존재하는지 vs PC 와의 연결만 안 된 것인지 구분한다.
			int32 PSActorCount = 0;
			FString PSNames;
			for (TActorIterator<APlayerState> PSIt(World); PSIt; ++PSIt)
			{
				++PSActorCount;
				PSNames += FString::Printf(TEXT("%s(%s) "), *PSIt->GetName(), *PSIt->GetClass()->GetName());
			}

			AGameStateBase* GS = World->GetGameState();
			UE_LOG(LogTemp, Warning,
				TEXT("[%s] GetLocalLOLPlayerState FAIL T=%.2f | World=%s | PC=%s LocalRole=%d RemoteRole=%d IsLocalPC=%d HasNetConn=%d | GameState=%s PlayerArrayNum=%d | PlayerStateActorsInWorld=%d [%s]"),
				NetModeStr,
				World->GetTimeSeconds(),
				*World->GetName(),
				*PC->GetName(), (int32)PC->GetLocalRole(), (int32)PC->GetRemoteRole(),
				PC->IsLocalPlayerController() ? 1 : 0, PC->GetNetConnection() ? 1 : 0,
				GS ? *GS->GetName() : TEXT("NULL"), GS ? GS->PlayerArray.Num() : -1,
				PSActorCount, *PSNames);
			return nullptr;
		}

		ALOLPlayerState* PS = Cast<ALOLPlayerState>(PC->PlayerState);
		if (PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] GetLocalLOLPlayerState SUCCESS T=%.2f | PS=%s Gold=%d"), NetModeStr, World->GetTimeSeconds(), *PS->GetName(), PS->GetGold());
		}
		if (!PS)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] GetLocalLOLPlayerState: PlayerState 가 ALOLPlayerState 가 아님. 실제 클래스=%s"), NetModeStr, *PC->PlayerState->GetClass()->GetName());
		}
		return PS;
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] GetLocalLOLPlayerState: 로컬 PlayerController 를 못 찾음 (순회한 PC 수=%d)"), NetModeStr, PCCount);
	return nullptr;
}
