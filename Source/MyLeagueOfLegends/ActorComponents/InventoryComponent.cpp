// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "ActorComponents/StatComponent.h"
#include "InGame/LOLPlayerState.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, Slots);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서 6칸 고정 초기화
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InitializeInventory();
	}
}

void UInventoryComponent::InitializeInventory()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// 멱등: 이미 6칸이면 스킵
	if (Slots.Num() != NUM_SLOTS)
	{
		Slots.Empty(NUM_SLOTS);
		Slots.SetNum(NUM_SLOTS); // FInventorySlot 기본값(빈 칸)으로 채움
		NotifyInventoryChanged();
	}
}

// ===================== 조회 =====================

FInventorySlot UInventoryComponent::GetSlot(int32 SlotIndex) const
{
	if (Slots.IsValidIndex(SlotIndex))
	{
		return Slots[SlotIndex];
	}
	return FInventorySlot();
}

FItemData UInventoryComponent::GetSlotItemData(int32 SlotIndex, bool& bFound) const
{
	bFound = false;
	FItemData Data;
	if (Slots.IsValidIndex(SlotIndex) && !Slots[SlotIndex].IsEmpty())
	{
		bFound = GetItemData(Slots[SlotIndex].Name, Data);
	}
	return Data;
}

bool UInventoryComponent::HasItem(FName RowName) const
{
	return FindSlotByName(RowName) != INDEX_NONE;
}

// ===================== 추가 (3-룰) =====================

EInventoryAddResult UInventoryComponent::AddItem(FName RowName)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return EInventoryAddResult::InvalidItem;
	}

	FItemData Item;
	if (!GetItemData(RowName, Item))
	{
		return EInventoryAddResult::InvalidItem;
	}

	if (Item.bIsStackable)
	{
		// 스택형: 단일 칸에만 쌓임, 중복 구매 불가
		const int32 Existing = FindSlotByName(RowName);
		if (Existing != INDEX_NONE)
		{
			if (Slots[Existing].Amount >= Item.MaxStackCount)
			{
				return EInventoryAddResult::MaxStackReached;
			}
			Slots[Existing].Amount++;
			ApplyItemStats(Item, true);
			NotifyInventoryChanged();
			return EInventoryAddResult::Success;
		}

		const int32 Empty = FindFirstEmptySlot();
		if (Empty == INDEX_NONE)
		{
			return EInventoryAddResult::InventoryFull;
		}
		Slots[Empty].Name = RowName;
		Slots[Empty].Amount = 1;
		ApplyItemStats(Item, true);
		NotifyInventoryChanged();
		return EInventoryAddResult::Success;
	}
	else if (Item.bIsDuplicatable)
	{
		// 중복형: 스택 안 함, 매번 새 칸에 추가
		const int32 Empty = FindFirstEmptySlot();
		if (Empty == INDEX_NONE)
		{
			return EInventoryAddResult::InventoryFull;
		}
		Slots[Empty].Name = RowName;
		Slots[Empty].Amount = 1;
		ApplyItemStats(Item, true);
		NotifyInventoryChanged();
		return EInventoryAddResult::Success;
	}
	else
	{
		// 단일형(Weapon): 하나만 소유 가능
		if (HasItem(RowName))
		{
			return EInventoryAddResult::AlreadyOwned;
		}
		const int32 Empty = FindFirstEmptySlot();
		if (Empty == INDEX_NONE)
		{
			return EInventoryAddResult::InventoryFull;
		}
		Slots[Empty].Name = RowName;
		Slots[Empty].Amount = 1;
		ApplyItemStats(Item, true);
		NotifyInventoryChanged();
		return EInventoryAddResult::Success;
	}
}

// ===================== 구매 =====================

bool UInventoryComponent::Server_RequestBuy_Validate(FName RowName)
{
	return true;
}

void UInventoryComponent::Server_RequestBuy_Implementation(FName RowName)
{
	FItemData Item;
	if (!GetItemData(RowName, Item))
	{
		return;
	}

	ALOLPlayerState* PS = GetOwningPlayerState();
	if (!PS)
	{
		return;
	}

	// 골드 부족 시 중단
	if (PS->GetGold() < Item.Price)
	{
		return;
	}

	// 규칙/칸 검증(AddItem)에 성공했을 때만 골드 차감
	const EInventoryAddResult Result = AddItem(RowName);
	if (Result == EInventoryAddResult::Success)
	{
		PS->SpendGold(Item.Price);
	}
}

// ===================== 판매/삭제 =====================

bool UInventoryComponent::Server_RequestSell_Validate(int32 SlotIndex)
{
	return true;
}

void UInventoryComponent::Server_RequestSell_Implementation(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		return;
	}

	FItemData Item;
	if (!GetItemData(Slots[SlotIndex].Name, Item))
	{
		return;
	}

	// 아이템 1개 제거 (0이 되면 빈 칸)
	Slots[SlotIndex].Amount--;
	if (Slots[SlotIndex].Amount <= 0)
	{
		Slots[SlotIndex] = FInventorySlot();
	}

	// 스탯 1개분 원복
	ApplyItemStats(Item, false);

	// 골드 환급
	if (ALOLPlayerState* PS = GetOwningPlayerState())
	{
		PS->AddGold(FMath::RoundToInt(Item.Price * SellRefundRatio));
	}

	NotifyInventoryChanged();
}

// ===================== 드래그 재배치 =====================

bool UInventoryComponent::Server_SwapSlots_Validate(int32 IndexA, int32 IndexB)
{
	return true;
}

void UInventoryComponent::Server_SwapSlots_Implementation(int32 IndexA, int32 IndexB)
{
	if (IndexA == IndexB || !Slots.IsValidIndex(IndexA) || !Slots.IsValidIndex(IndexB))
	{
		return;
	}

	Slots.Swap(IndexA, IndexB);
	NotifyInventoryChanged();
}

// ===================== 복제 콜백 =====================

void UInventoryComponent::OnRep_Slots()
{
	OnInventoryUpdated.Broadcast();
}

// ===================== 내부 헬퍼 =====================

int32 UInventoryComponent::FindSlotByName(FName RowName) const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (!Slots[i].IsEmpty() && Slots[i].Name == RowName)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindFirstEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool UInventoryComponent::GetItemData(FName RowName, FItemData& OutData) const
{
	if (!ItemDataTable || RowName.IsNone())
	{
		return false;
	}

	static const FString Ctx(TEXT("InventoryGetItemData"));
	if (const FItemData* Row = ItemDataTable->FindRow<FItemData>(RowName, Ctx))
	{
		OutData = *Row;
		return true;
	}
	return false;
}

UStatComponent* UInventoryComponent::GetStatComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UStatComponent>();
	}
	return nullptr;
}

ALOLPlayerState* UInventoryComponent::GetOwningPlayerState() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		return OwnerPawn->GetPlayerState<ALOLPlayerState>();
	}
	return nullptr;
}

void UInventoryComponent::ApplyItemStats(const FItemData& Item, bool bAdd)
{
	UStatComponent* Stat = GetStatComponent();
	if (!Stat)
	{
		return;
	}

	const FItemStat& S = Item.ItemStat;
	if (bAdd)
	{
		Stat->AddItemBonusStats(S.AD, S.AP, S.Defense, S.APDefense, S.MaxHP, S.MaxMP, S.MoveSpeed, S.AttackRate);
	}
	else
	{
		Stat->RemoveItemBonusStats(S.AD, S.AP, S.Defense, S.APDefense, S.MaxHP, S.MaxMP, S.MoveSpeed, S.AttackRate);
	}
}

void UInventoryComponent::NotifyInventoryChanged()
{
	// 리슨서버 호스트는 OnRep이 안 오므로 서버에서도 직접 broadcast
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OnInventoryUpdated.Broadcast();
	}
}
