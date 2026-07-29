// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemDataStructs.h"
#include "InventoryComponent.generated.h"

class UDataTable;
class UStatComponent;
class ALOLPlayerState;

// 인벤토리 변경 알림 (UI 갱신용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYLEAGUEOFLEGENDS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// 고정 칸 수
	static constexpr int32 NUM_SLOTS = 6;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ===== 델리게이트 =====
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryUpdated OnInventoryUpdated;

	// ===== 조회 (UI/BP용) =====
	// 슬롯 하나 반환
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FInventorySlot GetSlot(int32 SlotIndex) const;

	// 슬롯의 아이템 데이터(FItemData) 조회. bFound=false면 빈 칸/무효 (기존 BP GetSlotItemData 대체)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemData GetSlotItemData(int32 SlotIndex, bool& bFound) const;

	// 특정 아이템 보유 여부
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName RowName) const;

	// ===== 요청 (클라 -> 서버) =====
	// 구매: 골드 검증/차감 후 추가
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory")
	void Server_RequestBuy(FName RowName);
	void Server_RequestBuy_Implementation(FName RowName);
	bool Server_RequestBuy_Validate(FName RowName);

	// 판매/삭제: 한 개 제거 + 골드 환급
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory")
	void Server_RequestSell(int32 SlotIndex);
	void Server_RequestSell_Implementation(int32 SlotIndex);
	bool Server_RequestSell_Validate(int32 SlotIndex);

	// 드래그 재배치: 두 슬롯 교환
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory")
	void Server_SwapSlots(int32 IndexA, int32 IndexB);
	void Server_SwapSlots_Implementation(int32 IndexA, int32 IndexB);
	bool Server_SwapSlots_Validate(int32 IndexA, int32 IndexB);

	// ===== 서버 로직 (BP에서도 직접 호출 가능) =====
	// 골드와 무관하게 순수 추가. 3-룰(스택/중복/단일) 적용. 서버 authority 전용
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	EInventoryAddResult AddItem(FName RowName);

	// 6칸 빈 슬롯으로 초기화 (서버 전용, 멱등). 기존 BP 호출부 호환용
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory();

protected:
	UFUNCTION()
	void OnRep_Slots();

	// 아이템 데이터 테이블 (DT_ItemDataes 지정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UDataTable> ItemDataTable;

	// 판매 환급 비율 (0.7 = 70%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float SellRefundRatio = 0.7f;

	// 6칸 인벤토리 (항상 크기 6, 빈 칸은 Name=None)
	UPROPERTY(ReplicatedUsing = OnRep_Slots, VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> Slots;

private:
	// ===== 내부 헬퍼 =====
	int32 FindSlotByName(FName RowName) const;
	int32 FindFirstEmptySlot() const;

	// DataTable에서 행 조회. 성공 시 true
	bool GetItemData(FName RowName, FItemData& OutData) const;

	// 소유 폰의 StatComponent / PlayerState 접근
	UStatComponent* GetStatComponent() const;
	ALOLPlayerState* GetOwningPlayerState() const;

	// 스탯 가감 (아이템 1개분)
	void ApplyItemStats(const FItemData& Item, bool bAdd);

	// 서버에서 변경 후 호출: 복제 dirty + 리슨서버 즉시 broadcast
	void NotifyInventoryChanged();
};
