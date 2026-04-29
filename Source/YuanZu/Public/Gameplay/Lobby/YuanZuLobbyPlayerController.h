#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuLobbyPlayerController.generated.h"

class UUserWidget;
class AYuanZuLobbyPlayerState;

UCLASS()
class YUANZU_API AYuanZuLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AYuanZuLobbyPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	//大厅玩家状态
	UPROPERTY()
	TObjectPtr<AYuanZuLobbyPlayerState>YuanZuLobbyPlayerState;
	//加载界面类
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UUserWidget> LoadingWidgetClass;
	//记录已创建的加载控件，避免重复创建。
	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidgetInstance = nullptr;

public:
	//玩家加入房间提示
	UFUNCTION(Server, Reliable)
	void ServerSetPlayerNameFromClient(const FString& InPlayerName);
	//服务器请求占用队伍插槽位置
	UFUNCTION(Server, Reliable)
	void ServerRequestOccupyTeamSlot(ETeamType TeamType, int32 SlotIndex);
	//服务器设置准备状态
	UFUNCTION(Server, Reliable)
	void ServerSetReadyState(bool bInReady);
	//服务器发送聊天消息
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& InMessage);
	//房主踢出玩家
	UFUNCTION(Server, Reliable)
	void ServerRequestKickPlayer(int32 TargetPlayerId);
	//房主设置禁言玩家
	UFUNCTION(Server, Reliable)
	void ServerRequestToggleMutePlayer(int32 TargetPlayerId);
	//房主开始游戏
	UFUNCTION(Server, Reliable)
	void ServerRequestStartGame(const FString& InMapPath);
	//退出队伍
	UFUNCTION(Server, Reliable)
	void ServerClearSlot();
	//客户端显示加载面板
	UFUNCTION(Client, Reliable)
	void ClientShowLoadingWidget();

};
