#pragma once

#include "CoreMinimal.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuChatMessageInfo.generated.h"

USTRUCT(BlueprintType)
struct FYuanZuChatMessageInfo
{
	GENERATED_BODY()

public:
	//玩家名称
	UPROPERTY()
	FString PlayerName;
	//消息文本
	UPROPERTY()
	FString MessageText;
	//时间字符串
	UPROPERTY()
	FString TimeString;
	//用来区分普通聊天消息和玩家状态提示消息。
	UPROPERTY()
	bool bIsPlayerStateHint = false;
	//玩家属于哪个队伍
	UPROPERTY()
	ETeamType TeamType = ETeamType::ETT_None;
	//发送消息的玩家
	UPROPERTY()
	int32 SenderPlayerId = INDEX_NONE;
};