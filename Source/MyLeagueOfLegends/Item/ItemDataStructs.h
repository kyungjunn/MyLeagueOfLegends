#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" // 렌더링 및 FTableRowBase를 쓰기 위해 필수!
#include "ItemDataStructs.generated.h" // ⚠️내 파일 이름 뒤에 .generated.h를 꼭 붙여야 합니다!

// 1. 아이템 타입 열거형 (기존 E_ItemType 대체)
UENUM(BlueprintType)
enum class EItemType : uint8
{
    Potion      UMETA(DisplayName = "Potion"),
    Weapon      UMETA(DisplayName = "Weapon"),
    Shoes       UMETA(DisplayName = "Shoes"),
    Ward        UMETA(DisplayName = "Ward"),
    Normal      UMETA(DisplayName = "Normal")
};

// 2. 세부 스탯 구조체 (기존 ST_ItemStat 대체)
USTRUCT(BlueprintType)
struct FItemStat
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float AD = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float AP = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float Defense = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float APDefense = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float MoveSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float AttackRate = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float MaxHP = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float MaxMP = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float HPRegen = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat") float MPRegen = 0.f;
};

// 3. 전체 아이템 데이터 구조체 (기존 ST_ItemData 대체)
// 데이터 테이블의 행으로 쓰이려면 반드시 : public FTableRowBase 를 붙여야 합니다!
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") TObjectPtr<UTexture2D> Image;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") int32 Price = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") EItemType Type = EItemType::Normal;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") bool bIsStackable = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") bool bIsDuplicatable = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") int32 MaxStackCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item") FItemStat ItemStat; // 위에서 만든 스탯 구조체 중첩
};