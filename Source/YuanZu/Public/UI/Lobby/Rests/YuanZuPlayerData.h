#pragma once

#include "CoreMinimal.h"
#include "YuanZuPlayerData.generated.h"

USTRUCT(BlueprintType)
struct FYuanZuPlayerData
{
    GENERATED_BODY()

public:
    //玩家ID
    UPROPERTY()
    int32 PlayerId = INDEX_NONE;
    //玩家名称
    UPROPERTY()
    FString PlayerName;
    //玩家是否在队伍中
    UPROPERTY()
    bool bInTeam = false;
};