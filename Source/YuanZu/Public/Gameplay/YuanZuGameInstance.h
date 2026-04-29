// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuGameInstance.generated.h"

UCLASS()
class YUANZU_API UYuanZuGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	//»º´æÍæ¼Ò¶ÓÎé
	TMap<FString, ETeamType>PlayerTeams;

public:
	//»º´æÍæ¼Ò¶ÓÎé
	void CachePlayerTeams(const FString& PlayerID, ETeamType InTeamType);
	bool GetPlayerTeams(const FString& PlayerID, ETeamType& OutTeamType)const;
	void ClearPlayerTeams();

};
