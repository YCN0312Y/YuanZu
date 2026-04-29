// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Lobby/YuanZuLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void AYuanZuLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYuanZuLobbyPlayerState, bIsRoomOwner);
	DOREPLIFETIME(AYuanZuLobbyPlayerState, TeamType);
	DOREPLIFETIME(AYuanZuLobbyPlayerState, bReady);
	DOREPLIFETIME(AYuanZuLobbyPlayerState, SlotIndex);
}
