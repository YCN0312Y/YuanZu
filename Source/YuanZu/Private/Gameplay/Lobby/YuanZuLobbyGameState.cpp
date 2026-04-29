#include "Gameplay/Lobby/YuanZuLobbyGameState.h"

#include "Gameplay/Lobby/YuanZuLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void AYuanZuLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYuanZuLobbyGameState, TeamSlots);
	DOREPLIFETIME(AYuanZuLobbyGameState, ChatMessages);
	DOREPLIFETIME(AYuanZuLobbyGameState, PlayerJoinHints);
	DOREPLIFETIME(AYuanZuLobbyGameState, SelectedMapName);
	DOREPLIFETIME(AYuanZuLobbyGameState, LobbyPlayers);
}

void AYuanZuLobbyGameState::InitTeamSlots(int32 TotalPlayerCount)
{
	if (!HasAuthority()) return;

	TeamSlots.Empty();

	const int32 RedCount = (TotalPlayerCount + 1) / 2;
	const int32 BlueCount = TotalPlayerCount / 2;

	for (int32 i = 0; i < RedCount; i++)
	{
		FYuanZuTeamSlotInfo Slot;
		Slot.TeamType = ETeamType::ETT_Red;
		Slot.SlotIndex = i;
		TeamSlots.Add(Slot);
	}

	for (int32 i = 0; i < BlueCount; i++)
	{
		FYuanZuTeamSlotInfo Slot;
		Slot.TeamType = ETeamType::ETT_Blue;
		Slot.SlotIndex = i;
		TeamSlots.Add(Slot);
	}

	NotifySlotsChanged();
}

void AYuanZuLobbyGameState::ClearPlayerOldSlot(APlayerState* RequestPlayerState)
{
	AYuanZuLobbyPlayerState* YuanZuLPS = Cast<AYuanZuLobbyPlayerState>(RequestPlayerState);
	if (YuanZuLPS)
	{
		YuanZuLPS->TeamType = ETeamType::ETT_None;
		YuanZuLPS->SlotIndex = INDEX_NONE;
		YuanZuLPS->bReady = false;
	}

	for (FYuanZuTeamSlotInfo& Slot : TeamSlots)
	{
		if (Slot.OccupyingPlayer == RequestPlayerState)
		{
			Slot.bReady = false;
			Slot.bOccupied = false;
			Slot.PlayerName.Empty();
			Slot.bIsRoomOwner = false;
			Slot.OccupyingPlayer = nullptr;
		}
	}
}

bool AYuanZuLobbyGameState::ServerOccupySlot(APlayerState* RequestPlayerState, const FString& PlayerName, ETeamType TeamType, int32 SlotIndex, bool bStartReady)
{
	if (!RequestPlayerState) return false;

	FYuanZuTeamSlotInfo* TargetSlot = nullptr;
	for (FYuanZuTeamSlotInfo& Slot : TeamSlots)
	{
		if (Slot.TeamType == TeamType && Slot.SlotIndex == SlotIndex)
		{
			TargetSlot = &Slot;
			break;
		}
	}

	if (!TargetSlot) return false;
	if (TargetSlot->bOccupied && TargetSlot->OccupyingPlayer != RequestPlayerState)
	{
		return false;
	}
	if (TargetSlot->OccupyingPlayer == RequestPlayerState)
	{
		return true;
	}

	AYuanZuLobbyPlayerState* YuanZuLPS = Cast<AYuanZuLobbyPlayerState>(RequestPlayerState);
	ClearPlayerOldSlot(RequestPlayerState);

	TargetSlot->bOccupied = true;
	TargetSlot->PlayerName = PlayerName;
	TargetSlot->bIsRoomOwner = YuanZuLPS ? YuanZuLPS->bIsRoomOwner : false;
	TargetSlot->bReady = bStartReady;
	TargetSlot->OccupyingPlayer = RequestPlayerState;

	if (YuanZuLPS)
	{
		YuanZuLPS->TeamType = TeamType;
		YuanZuLPS->SlotIndex = SlotIndex;
		YuanZuLPS->bReady = bStartReady;
		SetLobbyPlayerInTeam(YuanZuLPS->GetPlayerId(), true);
	}

	NotifySlotsChanged();
	return true;
}

void AYuanZuLobbyGameState::SetPlayerReadyState(APlayerState* RequestPlayerState, bool bInReady)
{
	if (!HasAuthority() || !RequestPlayerState) return;

	AYuanZuLobbyPlayerState* YuanZuLPS = Cast<AYuanZuLobbyPlayerState>(RequestPlayerState);
	if (YuanZuLPS)
	{
		YuanZuLPS->bReady = bInReady;
	}

	for (FYuanZuTeamSlotInfo& Slot : TeamSlots)
	{
		if (Slot.OccupyingPlayer == RequestPlayerState)
		{
			Slot.bReady = bInReady;
			NotifySlotsChanged();
			return;
		}
	}
}

void AYuanZuLobbyGameState::SetSelectedMapName(const FString& InSelectedMapName)
{
	if (!HasAuthority()) return;

	SelectedMapName = InSelectedMapName;
	NotifySelectedMapChanged();
}

void AYuanZuLobbyGameState::AddChatMessage(APlayerState* SenderPlayerState, const FString& InMessage)
{
	if (!HasAuthority() || !SenderPlayerState) return;

	const FString TrimmedMessage = InMessage.TrimStartAndEnd();
	if (TrimmedMessage.IsEmpty()) return;

	FYuanZuChatMessageInfo NewMessage;
	NewMessage.PlayerName = SenderPlayerState->GetPlayerName();
	NewMessage.MessageText = TrimmedMessage.Left(200);
	NewMessage.TimeString = FDateTime::Now().ToString(TEXT("%H:%M"));
	NewMessage.SenderPlayerId = SenderPlayerState->GetPlayerId();
	NewMessage.TeamType = ETeamType::ETT_None;

	for (const FYuanZuTeamSlotInfo& Slot : TeamSlots)
	{
		if (Slot.OccupyingPlayer == SenderPlayerState)
		{
			NewMessage.TeamType = Slot.TeamType;
			break;
		}
	}

	ChatMessages.Add(NewMessage);

	const int32 MaxMessageCount = 50;
	if (ChatMessages.Num() > MaxMessageCount)
	{
		const int32 RemoveCount = ChatMessages.Num() - MaxMessageCount;
		ChatMessages.RemoveAt(0, RemoveCount);
	}

	NotifyChatMessagesChanged();
}

void AYuanZuLobbyGameState::AddPlayerStateChatHint(const FString& InPlayerName, const FString& InStateText)
{
	if (!HasAuthority()) return;

	const FString TrimmedPlayerName = InPlayerName.TrimStartAndEnd();
	const FString TrimmedStateText = InStateText.TrimStartAndEnd();
	if (TrimmedPlayerName.IsEmpty() || TrimmedStateText.IsEmpty()) return;

	FYuanZuChatMessageInfo NewMessage;
	NewMessage.PlayerName = TrimmedPlayerName;
	NewMessage.MessageText = TrimmedStateText;
	NewMessage.TimeString = FDateTime::Now().ToString(TEXT("%H:%M"));
	NewMessage.bIsPlayerStateHint = true;
	NewMessage.TeamType = ETeamType::ETT_None;
	NewMessage.SenderPlayerId = INDEX_NONE;

	ChatMessages.Add(NewMessage);

	const int32 MaxMessageCount = 50;
	if (ChatMessages.Num() > MaxMessageCount)
	{
		const int32 RemoveCount = ChatMessages.Num() - MaxMessageCount;
		ChatMessages.RemoveAt(0, RemoveCount);
	}

	NotifyChatMessagesChanged();
}

void AYuanZuLobbyGameState::AddPlayerJoinHint(const FString& InPlayerName, const FString& InStateText)
{
	if (!HasAuthority()) return;

	//接收玩家名称和玩家大厅状态
	const FString TrimmedPlayerName = InPlayerName.TrimStartAndEnd();
	const FString TrimmedStateText = InStateText.TrimStartAndEnd();
	if (TrimmedPlayerName.IsEmpty() || TrimmedStateText.IsEmpty()) return;

	//设置新提示
	FYuanZuLobbyHintInfo NewHint;
	NewHint.PlayerName = TrimmedPlayerName;
	NewHint.StateText = TrimmedStateText;
	NewHint.TimeString = FDateTime::Now().ToString(TEXT("%H:%M"));

	//将新提示添加到数组中
	PlayerJoinHints.Add(NewHint);

	const int32 MaxHintCount = 20;
	if (PlayerJoinHints.Num() > MaxHintCount)
	{
		const int32 RemoveCount = PlayerJoinHints.Num() - MaxHintCount;
		PlayerJoinHints.RemoveAt(0, RemoveCount);
	}

	NotifyPlayerJoinHintsChanged();
}

void AYuanZuLobbyGameState::RemovePlayerFromSlot(APlayerState* LeavingPlayerState)
{
	if (!HasAuthority() || !LeavingPlayerState) return;

	ClearPlayerOldSlot(LeavingPlayerState);
	NotifySlotsChanged();
	RemoveLobbyPlayer(LeavingPlayerState->GetPlayerId());
}

void AYuanZuLobbyGameState::QuitTeam(APlayerState* PS)
{
	if (!HasAuthority() || !PS) return;

	ClearPlayerOldSlot(PS);
	NotifySlotsChanged();
	SetLobbyPlayerInTeam(PS->GetPlayerId(), false);
}

bool AYuanZuLobbyGameState::IsPlayerTeam(APlayerState* InLPS) const
{
	if (!InLPS) return false;

	for (const FYuanZuTeamSlotInfo& Slot : TeamSlots)
	{
		if (Slot.OccupyingPlayer == InLPS)
		{
			return true;
		}
	}

	return false;
}

void AYuanZuLobbyGameState::OnRep_TeamSlots()
{
	NotifySlotsChanged();
}

void AYuanZuLobbyGameState::OnRep_SelectedMapName()
{
	NotifySelectedMapChanged();
}

void AYuanZuLobbyGameState::OnRep_ChatMessages()
{
	NotifyChatMessagesChanged();
}

void AYuanZuLobbyGameState::OnRep_PlayerJoinHints()
{
	NotifyPlayerJoinHintsChanged();
}

void AYuanZuLobbyGameState::OnRep_LobbyPlayers()
{
	NotifyPlayerListChanged();
}

void AYuanZuLobbyGameState::NotifySlotsChanged()
{
	OnTeamSlotsChanged.Broadcast(TeamSlots);
}

void AYuanZuLobbyGameState::NotifyPlayerListChanged()
{
	OnPlayerListChanged.Broadcast();
}

void AYuanZuLobbyGameState::NotifySelectedMapChanged()
{
	OnSelectedMapChanged.Broadcast(SelectedMapName);
}

void AYuanZuLobbyGameState::NotifyChatMessagesChanged()
{
	OnChatMessagesChanged.Broadcast(ChatMessages);
}

void AYuanZuLobbyGameState::NotifyPlayerJoinHintsChanged()
{
	OnPlayerJoinHintsChanged.Broadcast(PlayerJoinHints);
}

void AYuanZuLobbyGameState::AddLobbyPlayer(int32 PlayerId, const FString& PlayerName)
{
	if (!HasAuthority() || PlayerId == INDEX_NONE) return;

	//获取玩家名称
	const FString TrimmedName = PlayerName.TrimStartAndEnd();
	if (TrimmedName.IsEmpty()) return;

	//加入大厅先遍历一下这个玩家ID是否在大厅玩家的数据中
	for (FYuanZuPlayerData& PlayerData : LobbyPlayers)
	{
		if (PlayerData.PlayerId == PlayerId)
		{
			PlayerData.PlayerName = TrimmedName;
			NotifyPlayerListChanged();
			return;
		}
	}

	//不在就创建一个玩家数据 接收这个玩家ID和名称然后刷线列表
	FYuanZuPlayerData NewPlayerData;
	NewPlayerData.PlayerId = PlayerId;
	NewPlayerData.PlayerName = TrimmedName;
	NewPlayerData.bInTeam = false;
	LobbyPlayers.Add(NewPlayerData);
	NotifyPlayerListChanged();
}

void AYuanZuLobbyGameState::SetLobbyPlayerInTeam(int32 PlayerId, bool bInTeam)
{
	if (!HasAuthority() || PlayerId == INDEX_NONE) return;

	for (FYuanZuPlayerData& PlayerData : LobbyPlayers)
	{
		if (PlayerData.PlayerId == PlayerId)
		{
			if (PlayerData.bInTeam == bInTeam)
			{
				return;
			}

			PlayerData.bInTeam = bInTeam;
			NotifyPlayerListChanged();
			return;
		}
	}
}

void AYuanZuLobbyGameState::RemoveLobbyPlayer(int32 PlayerId)
{
	if (!HasAuthority() || PlayerId == INDEX_NONE) return;

	const int32 RemovedCount = LobbyPlayers.RemoveAll(
		[PlayerId](const FYuanZuPlayerData& PlayerData)
		{
			return PlayerData.PlayerId == PlayerId;
		}
	);

	if (RemovedCount > 0)
	{
		NotifyPlayerListChanged();
	}
}
