#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "UI/Lobby/Rests/YuanZuChatMessageInfo.h"
#include "UI/Lobby/Rests/YuanZuLobbyHintInfo.h"
#include "UI/Lobby/Rests/YuanZuPlayerData.h"
#include "YuanZuLobbyGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTeamSlotsChanged, const TArray<FYuanZuTeamSlotInfo>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChatMessagesChanged, const TArray<FYuanZuChatMessageInfo>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectedMapChanged, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerJoinHintsChanged, const TArray<FYuanZuLobbyHintInfo>&);
DECLARE_MULTICAST_DELEGATE(FOnPlayerListChanged);

UCLASS()
class YUANZU_API AYuanZuLobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	//队伍插槽
	UPROPERTY(ReplicatedUsing = OnRep_TeamSlots)
	TArray<FYuanZuTeamSlotInfo> TeamSlots;
	//聊天消息
	UPROPERTY(ReplicatedUsing = OnRep_ChatMessages)
	TArray<FYuanZuChatMessageInfo> ChatMessages;
	//玩家加入提示
	UPROPERTY(ReplicatedUsing = OnRep_PlayerJoinHints)
	TArray<FYuanZuLobbyHintInfo> PlayerJoinHints;
	//选择的地图
	UPROPERTY(ReplicatedUsing = OnRep_SelectedMapName)
	FString SelectedMapName;
	//玩家
	UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayers)
	TArray<FYuanZuPlayerData> LobbyPlayers;

public:
	FOnTeamSlotsChanged OnTeamSlotsChanged;
	FOnSelectedMapChanged OnSelectedMapChanged;
	FOnChatMessagesChanged OnChatMessagesChanged;
	FOnPlayerJoinHintsChanged OnPlayerJoinHintsChanged;
	FOnPlayerListChanged OnPlayerListChanged;

protected:
	//队伍插槽
	UFUNCTION()
	void OnRep_TeamSlots();
	//选择的地图
	UFUNCTION()
	void OnRep_SelectedMapName();
	//聊天消息
	UFUNCTION()
	void OnRep_ChatMessages();
	//玩家加入提示
	UFUNCTION()
	void OnRep_PlayerJoinHints();
	//玩家
	UFUNCTION()
	void OnRep_LobbyPlayers();

private:
	//执行选择地图代理
	void NotifySelectedMapChanged();
	//执行聊天消息代理
	void NotifyChatMessagesChanged();
	//执行玩家加入提示代理
	void NotifyPlayerJoinHintsChanged();
	//清除玩家老插槽
	void ClearPlayerOldSlot(APlayerState* RequestPlayerState);

public:
	//初始化队伍插槽
	void InitTeamSlots(int32 TotalPlayerCount);
	//占用插槽
	bool ServerOccupySlot(APlayerState* RequestPlayerState, const FString& PlayerName, ETeamType TeamType, int32 SlotIndex, bool bStartReady);
	//设置玩家准备状态
	void SetPlayerReadyState(APlayerState* RequestPlayerState, bool bInReady);
	//设置选择的地图
	void SetSelectedMapName(const FString& InSelectedMapName);
	//添加聊天消息
	void AddChatMessage(APlayerState* SenderPlayerState, const FString& InMessage);
	//添加玩家玩家是否被禁言提示
	void AddPlayerStateChatHint(const FString& InPlayerName, const FString& InStateText);
	//添加玩家加入提示
	void AddPlayerJoinHint(const FString& InPlayerName, const FString& InStateText);
	//踢出玩家
	void RemovePlayerFromSlot(APlayerState* LeavingPlayerState);
	//退出队伍
	void QuitTeam(APlayerState* PS);
	//玩家是否在插槽
	bool IsPlayerTeam(APlayerState* InLPS) const;
	//执行插槽被占用代理
	void NotifySlotsChanged();
	//更新玩家列表代理
	void NotifyPlayerListChanged();
	//大厅添加玩家
	void AddLobbyPlayer(int32 PlayerId, const FString& PlayerName);
	//设置大厅玩家队伍
	void SetLobbyPlayerInTeam(int32 PlayerId, bool bInTeam);
	//列表移除玩家
	void RemoveLobbyPlayer(int32 PlayerId);

	const FString& GetSelectedMapName() const { return SelectedMapName; }
	const TArray<FYuanZuTeamSlotInfo>& GetTeamSlots() const { return TeamSlots; }
	const TArray<FYuanZuChatMessageInfo>& GetChatMessages() const { return ChatMessages; }
	const TArray<FYuanZuLobbyHintInfo>& GetPlayerJoinHints() const { return PlayerJoinHints; }
	const TArray<FYuanZuPlayerData>& GetLobbyPlayers() const { return LobbyPlayers; }
};
