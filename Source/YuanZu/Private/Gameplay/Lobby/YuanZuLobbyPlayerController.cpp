#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/Lobby/YuanZuLobbyGameMode.h"
#include "Gameplay/Lobby/YuanZuLobbyGameState.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerState.h"
#include "MultiplayerSessionsSubsystem.h"

AYuanZuLobbyPlayerController::AYuanZuLobbyPlayerController()
{
}

void AYuanZuLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UMultiplayerSessionsSubsystem* MSSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if (!MSSubsystem) return;

	const FString CachedPlayerName = MSSubsystem->GetCachedPlayerName();
	if (CachedPlayerName.IsEmpty()) return;

	ServerSetPlayerNameFromClient(CachedPlayerName);
}

//玩家加入房间提示
void AYuanZuLobbyPlayerController::ServerSetPlayerNameFromClient_Implementation(const FString& InPlayerName)
{
	AYuanZuLobbyPlayerState* YuanZuLPS = GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS) return;

	const FString TrimmedName = InPlayerName.TrimStartAndEnd();
	if (TrimmedName.IsEmpty()) return;

	YuanZuLPS->SetPlayerName(TrimmedName);

	AYuanZuLobbyGameState* YuanZuLGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (YuanZuLGS)
	{
		YuanZuLGS->AddPlayerJoinHint(TrimmedName, TEXT("加入游戏"));
		YuanZuLGS->AddLobbyPlayer(YuanZuLPS->GetPlayerId(), TrimmedName);
	}
}
//加入队伍
void AYuanZuLobbyPlayerController::ServerRequestOccupyTeamSlot_Implementation(ETeamType TeamType, int32 SlotIndex)
{
	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS) return;

	AYuanZuLobbyPlayerState* YuanZuLPS = GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS) return;

	const FString PlayerName = YuanZuLPS->GetPlayerName();
	if (PlayerName.IsEmpty()) return;

	AYuanZuLobbyGameMode* LobbyGM = GetWorld() ? GetWorld()->GetAuthGameMode<AYuanZuLobbyGameMode>() : nullptr;
	const bool bStartReady = LobbyGM && LobbyGM->IsRoomOwner(this);

	LobbyGS->ServerOccupySlot(YuanZuLPS, PlayerName, TeamType, SlotIndex, bStartReady);
}
//服务器设置准备状态
void AYuanZuLobbyPlayerController::ServerSetReadyState_Implementation(bool bInReady)
{
	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS) return;

	AYuanZuLobbyPlayerState* YuanZuLPS = GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS) return;

	LobbyGS->SetPlayerReadyState(YuanZuLPS, bInReady);
}
//服务器发送聊天消息
void AYuanZuLobbyPlayerController::ServerSendChatMessage_Implementation(const FString& InMessage)
{
	AYuanZuLobbyGameMode* LobbyGM = GetWorld() ? GetWorld()->GetAuthGameMode<AYuanZuLobbyGameMode>() : nullptr;
	if (!LobbyGM) return;

	AYuanZuLobbyPlayerState* YuanZuLPS = GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS) return;

	if (LobbyGM->IsPlayerMuted(YuanZuLPS->GetPlayerId()))
	{
		return;
	}

	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS) return;

	LobbyGS->AddChatMessage(YuanZuLPS, InMessage);
}
//房主踢出玩家
void AYuanZuLobbyPlayerController::ServerRequestKickPlayer_Implementation(int32 TargetPlayerId)
{
	AYuanZuLobbyGameMode* LobbyGM = GetWorld() ? GetWorld()->GetAuthGameMode<AYuanZuLobbyGameMode>() : nullptr;
	if (!LobbyGM) return;

	LobbyGM->RemovePlayerById(this, TargetPlayerId);
}
//房主设置禁言玩家
void AYuanZuLobbyPlayerController::ServerRequestToggleMutePlayer_Implementation(int32 TargetPlayerId)
{
	AYuanZuLobbyGameMode* LobbyGM = GetWorld() ? GetWorld()->GetAuthGameMode<AYuanZuLobbyGameMode>() : nullptr;
	if (!LobbyGM) return;

	LobbyGM->ToggleMutePlayerById(this, TargetPlayerId);
}
//房主开始游戏
void AYuanZuLobbyPlayerController::ServerRequestStartGame_Implementation(const FString& InMapPath)
{
	AYuanZuLobbyGameMode* LobbyGM = GetWorld() ? GetWorld()->GetAuthGameMode<AYuanZuLobbyGameMode>() : nullptr;
	if (!LobbyGM) return;

	LobbyGM->RequestStartGame(this, InMapPath);
}
//退出队伍
void AYuanZuLobbyPlayerController::ServerClearSlot_Implementation()
{
	AYuanZuLobbyGameState* YuanZuLGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!YuanZuLGS) return;

	AYuanZuLobbyPlayerState* YuanZuLPS = GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS) return;

	YuanZuLGS->QuitTeam(YuanZuLPS);
}
//客户端显示加载面板
void AYuanZuLobbyPlayerController::ClientShowLoadingWidget_Implementation()
{
	if (!LoadingWidgetClass || LoadingWidgetInstance) return;

	LoadingWidgetInstance = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->AddToViewport(1000);
	}
}
