// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuLobbyPlayerState.generated.h"

UCLASS()
class YUANZU_API AYuanZuLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:
	//是否是房主
	UPROPERTY(Replicated)
	bool bIsRoomOwner = false;
	//所属阵营
	UPROPERTY(Replicated)
	ETeamType TeamType = ETeamType::ETT_None;
	//玩家是否已准备
	UPROPERTY(Replicated)
	bool bReady = false;
	//插槽索引
	UPROPERTY(Replicated)
	int32 SlotIndex = INDEX_NONE;
};
