// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"


#include "LOLPlayerState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerSelectionChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None		UMETA(DisplayName = "None"),
	Blue		UMETA(DisplayName = "BlueTeam"),
	Red			UMETA(DisplayName = "RedTeam")
};

class ULOLGameInstance;

/**
 * 
 */
UCLASS()
class MYLEAGUEOFLEGENDS_API ALOLPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ALOLPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 팀 설정
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(ETeam NewTeam) { if (HasAuthority()) Team = NewTeam; }

	UFUNCTION(BlueprintPure, Category = "Team")
	ETeam GetTeam() const { return Team; }

	// 고른 챔피언 데이터 테이블
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Select")
	void C2S_SelectChampion(FName ChampionRowName);
	void C2S_SelectChampion_Implementation(FName ChampionRowName);

	UFUNCTION(BlueprintPure, Category = "Select")
	FName GetSelectedChampion() const { return SelectedChampion; }

	UFUNCTION(BlueprintCallable, Category = "Select")
	void SetSelectedChampion(FName NewChampion) { if (HasAuthority()) SelectedChampion = NewChampion; }

	// 준비 완료 상태 관리
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Select")
	void Server_ReadyUp();
	void Server_ReadyUp_Implementation();

	UFUNCTION(BlueprintPure, Category = "Select")
	bool IsReady() const { return bIsReady; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Select|Events")
	FOnPlayerSelectionChanged OnSelectionChanged;

	// ===== 골드 (서버 권위) =====
	UPROPERTY(BlueprintAssignable, Category = "Gold|Events")
	FOnGoldChanged OnGoldChanged;

	UFUNCTION(BlueprintPure, Category = "Gold")
	int32 GetGold() const { return Gold; }

	// 서버 전용: 골드 차감(부족하면 false). 인벤토리 구매에서 호출
	bool SpendGold(int32 Amount);

	// 서버 전용: 골드 획득/환급
	void AddGold(int32 Amount);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Team, VisibleAnywhere, BlueprintReadOnly, Category = "Team")
	ETeam Team;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedChampion, VisibleAnywhere, BlueprintReadOnly, Category = "Select")
	FName SelectedChampion;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Select")
	bool bIsReady;

	UPROPERTY(ReplicatedUsing = OnRep_Gold, VisibleAnywhere, BlueprintReadOnly, Category = "Gold")
	int32 Gold;

	// 시작 골드 (에디터에서 조절 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gold")
	int32 InitialGold = 500;

	UFUNCTION() 
	void OnRep_Team();
	UFUNCTION() 
	void OnRep_SelectedChampion();

	UFUNCTION()
	void OnRep_Gold();

private:
	void SaveToLocalGameInstance();
};
