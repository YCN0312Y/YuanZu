#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "YuanZuTeamSlotInfo.generated.h"

UENUM(BlueprintType)
enum class ETeamType : uint8
{
	ETT_None UMETA(DisplayName = "None"),
	ETT_Red UMETA(DisplayName = "Red"),
	ETT_Blue UMETA(DisplayName = "Bule"),
	ETT_Max UMETA(DisplayName = "Max")
};

USTRUCT(BlueprintType)
struct FYuanZuTeamSlotInfo
{
	GENERATED_BODY()

public:
	//所属阵营
	UPROPERTY()
	ETeamType TeamType = ETeamType::ETT_None;
	//插槽索引
	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;
	//插槽是否已占有
	UPROPERTY()
	bool bOccupied = false;
	//玩家名称
	UPROPERTY()
	FString PlayerName;
	//是否是房主
	UPROPERTY()
	bool bIsRoomOwner = false;
	//是否已准备
	UPROPERTY()
	bool bReady = false;
	//占用插槽的玩家状态
	UPROPERTY()
	TObjectPtr<APlayerState> OccupyingPlayer = nullptr;
};
