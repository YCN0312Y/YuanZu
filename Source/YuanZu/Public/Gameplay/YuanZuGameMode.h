// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "YuanZuGameMode.generated.h"

namespace MatchState
{
	//比赛时长已到显示胜方冷却对局
	extern YUANZU_API const FName Cooldown;
}

class AYuanZuCharacterBase;
class AYuanZuPlayerController;

UCLASS()
class YUANZU_API AYuanZuGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AYuanZuGameMode();

protected:
	virtual void BeginPlay() override;
	//比赛状态改变时调用
	virtual void OnMatchStateSet() override;
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void Tick(float DeltaTime);

public:
	//等待开始游戏时间
	UPROPERTY(EditDefaultsOnly, Category = YuanZu, meta = (ToolTip = "The warm-up period before the start of the competition."))
	float WarmupTime = 10.f;
	//比赛时长
	UPROPERTY(EditDefaultsOnly, Category = YuanZu, meta = (ToolTip = "Duration of the internal competition."))
	float MatchTime = 120.f;
	//比赛冷却时间
	UPROPERTY(EditDefaultsOnly, Category = YuanZu, meta = (ToolTip = "The waiting period before the start of the new competition."))
	float CooldownTime = 10.f;

	//关卡开始时间
	float LevelStartingTime = 0.f;	

private:
	//倒计时时间
	float CountdownTime = 0.f;

public:
	virtual void PlayerDeath(AYuanZuCharacterBase* DeathCharacter, AYuanZuPlayerController* DeathController, AYuanZuPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* DeathCharacter, AController* VictimController);

	void HandleHostQuitToLogin();
	void SyncSessionPlayerCount();

	FORCEINLINE float GetCooldownTime() const {return CooldownTime; }
};
