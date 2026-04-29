#include "Gameplay/Lobby/YuanZuLobbyGameMode.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/Lobby/YuanZuLobbyGameState.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerState.h"
#include "MultiplayerSessionsSubsystem.h"
#include "TimerManager.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "Gameplay/YuanZuGameInstance.h"

void AYuanZuLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	AYuanZuLobbyGameState* LobbyGS = GetGameState<AYuanZuLobbyGameState>();
	if (!LobbyGS)
	{
		UE_LOG(LogYuanZu, Error, TEXT("LobbyGS == nullptr"));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogYuanZu, Error, TEXT("GameInstance == nullptr"));
		return;
	}

	UMultiplayerSessionsSubsystem* MSSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if (!MSSubsystem)
	{
		UE_LOG(LogYuanZu, Error, TEXT("MSSubsystem == nullptr"));
		return;
	}

	const int32 CachedPlayerCount = MSSubsystem->GetCachedPlayerCount();
	UE_LOG(LogYuanZu, Warning, TEXT("RoomMaxPlayerCount = %d"), CachedPlayerCount);

	LobbyGS->InitTeamSlots(CachedPlayerCount);
}

void AYuanZuLobbyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AYuanZuLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AYuanZuLobbyPlayerController* YuanZuLPC = Cast<AYuanZuLobbyPlayerController>(NewPlayer);
	if (!YuanZuLPC) return;

	AYuanZuLobbyPlayerState* YuanZuLPS = YuanZuLPC->GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS) return;

	YuanZuLPS->bIsRoomOwner = IsRoomOwner(YuanZuLPC);
}

void AYuanZuLobbyGameMode::Logout(AController* Exiting)
{
	int32 LeavingPlayerID = INDEX_NONE;
	FString LeavingPlayerName;
	AYuanZuLobbyPlayerState* LeavingPlayerState = nullptr;
	if (Exiting && Exiting->PlayerState)
	{
		LeavingPlayerID = Exiting->PlayerState->GetPlayerId();
		LeavingPlayerName = Exiting->PlayerState->GetPlayerName();
		LeavingPlayerState = Cast<AYuanZuLobbyPlayerState>(Exiting->PlayerState);
	}

	AYuanZuLobbyGameState* YuanZuLGS = GetGameState<AYuanZuLobbyGameState>();

	Super::Logout(Exiting);

	if (YuanZuLGS && !LeavingPlayerName.IsEmpty())
	{
		YuanZuLGS->AddPlayerJoinHint(LeavingPlayerName, TEXT("离开了游戏"));
	}

	MutedPlayerIds.Remove(LeavingPlayerID);

	if (YuanZuLGS && LeavingPlayerState)
	{
		YuanZuLGS->RemovePlayerFromSlot(LeavingPlayerState);
	}
}

//根据玩家ID移除玩家
void AYuanZuLobbyGameMode::RemovePlayerById(AYuanZuLobbyPlayerController* Requester, int32 TargetPlayerId)
{
	if (!HasAuthority() || !Requester) return;
	if (!IsRoomOwner(Requester)) return;

	if (Requester->PlayerState && Requester->PlayerState->GetPlayerId() == TargetPlayerId)
	{
		return;
	}

	AYuanZuLobbyGameState* LobbyGS = GetGameState<AYuanZuLobbyGameState>();
	if (!LobbyGS) return;

	//遍历时间所有控制器
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AYuanZuLobbyPlayerController* TargetPC = Cast<AYuanZuLobbyPlayerController>(It->Get());
		if (!TargetPC || !TargetPC->PlayerState) continue;

		if (TargetPC->PlayerState->GetPlayerId() == TargetPlayerId)
		{
			//检查玩家是否真的离开
			const bool bKickIssued = GameSession && GameSession->KickPlayer(TargetPC, FText::FromString(TEXT(" ")));
			if (bKickIssued)
			{
				LobbyGS->RemovePlayerFromSlot(TargetPC->PlayerState);
			}
			break;
		}
	}
}

//根据玩家ID禁言玩家
void AYuanZuLobbyGameMode::ToggleMutePlayerById(AYuanZuLobbyPlayerController* Requester, int32 TargetPlayerId)
{
	if (!HasAuthority() || !Requester) return;
	if (!IsRoomOwner(Requester)) return;

	if (Requester->PlayerState && Requester->PlayerState->GetPlayerId() == TargetPlayerId)
	{
		return;
	}

	AYuanZuLobbyGameState* YuanZuLGS = GetGameState<AYuanZuLobbyGameState>();
	if (!YuanZuLGS) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AYuanZuLobbyPlayerController* TargetPC = Cast<AYuanZuLobbyPlayerController>(It->Get());
		if (!TargetPC || !TargetPC->PlayerState) continue;
		if (TargetPC->PlayerState->GetPlayerId() != TargetPlayerId) continue;

		if (MutedPlayerIds.Contains(TargetPlayerId))
		{
			MutedPlayerIds.Remove(TargetPlayerId);
			YuanZuLGS->AddPlayerStateChatHint(TargetPC->PlayerState->GetPlayerName(), TEXT("已解除禁言"));
		}
		else
		{
			MutedPlayerIds.Add(TargetPlayerId);
			YuanZuLGS->AddPlayerStateChatHint(TargetPC->PlayerState->GetPlayerName(), TEXT("已被禁言"));
		}
		break;
	}
}

//开始游戏
void AYuanZuLobbyGameMode::RequestStartGame(AYuanZuLobbyPlayerController* Requester, const FString& InMapPath)
{
	if (!HasAuthority() || !Requester) return;
	if (!IsRoomOwner(Requester)) return;
	if (bIsStartingGame) return;
	if (InMapPath.IsEmpty()) return;
	if (!AreAllOccupiedPlayersReady()) return;

	//开始游戏时检查待加入队伍的玩家
	RemovePlayersWithoutTeam(Requester);
	//
	if (!AreAllOccupiedPlayersReady()) return;

	bIsStartingGame = true;
	PendingTravelMapPath = InMapPath;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AYuanZuLobbyPlayerController* PC = Cast<AYuanZuLobbyPlayerController>(It->Get());
		if (PC)
		{
			PC->ClientShowLoadingWidget();
		}
	}
	//根据定时的时间加入游戏
	GetWorldTimerManager().SetTimer(StartGameTimerHandle, this, &AYuanZuLobbyGameMode::StartGameTravel, StartWaitTime, false);
}

//玩家是否已经被禁言
bool AYuanZuLobbyGameMode::IsPlayerMuted(int32 PlayerId) const
{
	//如果禁言的玩家在 被禁言的玩家列表中就返回true
	return MutedPlayerIds.Contains(PlayerId);
}

bool AYuanZuLobbyGameMode::IsRoomOwner(AYuanZuLobbyPlayerController* Requester) const
{
	//检查玩家是否是房主
	if (!Requester) return false;
	return Requester->IsLocalController();
}

void AYuanZuLobbyGameMode::RemovePlayersWithoutTeam(AYuanZuLobbyPlayerController* Requester)
{
	if (!HasAuthority() && !GetWorld())return;

	AYuanZuLobbyGameState* YuanZuLGS = GetWorld()->GetGameState<AYuanZuLobbyGameState>();
	if (!YuanZuLGS || !GameSession)return;

	//待移除的玩家列表
	TArray<AYuanZuLobbyPlayerController*> PlayersToKick;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		//遍历加入大厅的所有玩家并且转换
		AYuanZuLobbyPlayerController* TargetPC = Cast<AYuanZuLobbyPlayerController>(It->Get());
		if (!TargetPC || TargetPC->PlayerState)return;

		//如果是房主自己就不管
		if (TargetPC == Requester)continue;

		if (!YuanZuLGS->IsPlayerTeam(TargetPC->PlayerState))
		{		
			//将未加入队伍的玩家添加到 待移除的玩家列表
			PlayersToKick.Add(TargetPC);
		}
	}

	//遍历 待移除的玩家列表 将里面的玩家都移除
	for (AYuanZuLobbyPlayerController* TargetPC : PlayersToKick)
	{
		GameSession->KickPlayer(TargetPC, FText::FromString(TEXT("未加入阵营，已被移出房间")));
	}
}

//检查所有玩家是否已经都准备好了
bool AYuanZuLobbyGameMode::AreAllOccupiedPlayersReady() const
{
	const AYuanZuLobbyGameState* LobbyGS = GetGameState<AYuanZuLobbyGameState>();
	if (!LobbyGS) return false;

	bool bHasOccupiedPlayer = false;

	for (const FYuanZuTeamSlotInfo& SlotInfo : LobbyGS->GetTeamSlots())
	{
		//遍历所有插槽如果还有插槽未被占用就继续
		if (!SlotInfo.bOccupied)
		{
			continue;
		}

		bHasOccupiedPlayer = true;
		//如果还有玩家没准备就返回false
		if (!SlotInfo.bReady)
		{
			return false;
		}
	}
	//如果都准备好了就可以开始游戏了
	return bHasOccupiedPlayer;
}

void AYuanZuLobbyGameMode::StartGameTravel()
{
	if (!GetWorld() || PendingTravelMapPath.IsEmpty())
	{
		bIsStartingGame = false;
		return;
	}

	UYuanZuGameInstance* YuanZuGI = GetGameInstance<UYuanZuGameInstance>();
	if (YuanZuGI)
	{
		//开始游戏先清空缓存的玩家列表
		YuanZuGI->ClearPlayerTeams();

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
		{
			AYuanZuLobbyPlayerController* YuanZuLPC = Cast<AYuanZuLobbyPlayerController>(It->Get());
			if (YuanZuLPC)
			{
				AYuanZuLobbyPlayerState* YuanZuLPS = YuanZuLPC->GetPlayerState<AYuanZuLobbyPlayerState>();
				if (!YuanZuLPS || !YuanZuLPS->GetUniqueId().IsValid()) continue;

				//获取加入的玩家的唯一ID
				const FString PlayerJoinID = YuanZuLPS->GetUniqueId()->ToString();

				//将玩家信息缓存到GameInstance中
				YuanZuGI->CachePlayerTeams(PlayerJoinID, YuanZuLPS->TeamType);

				UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = YuanZuGI->GetSubsystem<UMultiplayerSessionsSubsystem>();
				if (MultiplayerSessionsSubsystem)
				{
					//房主开始游戏就把房间锁住防止其他玩家中途加入
					MultiplayerSessionsSubsystem->LockRoom();
				}
			}
		}
	}
	GetWorld()->ServerTravel(PendingTravelMapPath + TEXT("?listen"));
}
