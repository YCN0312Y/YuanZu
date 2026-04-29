// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "YuanZuLobbyGameMode.generated.h"

class AYuanZuLobbyPlayerController;

UCLASS()
class YUANZU_API AYuanZuLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//登录
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//登出
	virtual void Logout(AController* Exiting) override;

private:
	//禁言列表
	TSet<int32> MutedPlayerIds;
	//用一个短延迟给所有客户端留出显示加载界面的时间。
	FTimerHandle StartGameTimerHandle;
	//地图路径
	FString PendingTravelMapPath;
	//是否开始游戏
	bool bIsStartingGame = false;
	//玩家开始游戏等待时间
	UPROPERTY(EditAnywhere, Category = YuanZu)
	int32 StartWaitTime;

public:
	//根据玩家ID移除玩家
	void RemovePlayerById(AYuanZuLobbyPlayerController* Requester, int32 TargetPlayerId);
	//根据玩家ID禁言玩家
	void ToggleMutePlayerById(AYuanZuLobbyPlayerController* Requester, int32 TargetPlayerId);
	//根据玩家准备情况开始游戏按钮是否启用
	void RequestStartGame(AYuanZuLobbyPlayerController* Requester, const FString& InMapPath);
	//玩家是否已禁言
	bool IsPlayerMuted(int32 PlayerId) const;
	//是不是房主
	bool IsRoomOwner(AYuanZuLobbyPlayerController* Requester) const;

private:
	//移除未进入任何队伍的玩家
	void RemovePlayersWithoutTeam(AYuanZuLobbyPlayerController* Requester);
	//进入队伍的玩家都已经准备才能开始游戏
	bool AreAllOccupiedPlayersReady() const;
	//进入地图
	UFUNCTION(BlueprintCallable, Category = YuanZu)
	void StartGameTravel();
};
