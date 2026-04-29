// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/YuanZuGameInstance.h"

void UYuanZuGameInstance::CachePlayerTeams(const FString& PlayerID, ETeamType InTeamType)
{
	if (PlayerID.IsEmpty())return;
	//添加玩家的阵营设置
	PlayerTeams.Add(PlayerID, InTeamType);
}

bool UYuanZuGameInstance::GetPlayerTeams(const FString& PlayerID, ETeamType& OutTeamType) const
{
	//根据输入进来的ID查询是否在缓存玩家队伍中
	if (const ETeamType* FoundItem = PlayerTeams.Find(PlayerID))
	{
		OutTeamType = *FoundItem;
		return true;
	}
	return false;
}

void UYuanZuGameInstance::ClearPlayerTeams()
{
	//清空
	PlayerTeams.Empty();
}
