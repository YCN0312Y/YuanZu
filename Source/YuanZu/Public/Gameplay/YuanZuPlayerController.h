// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiplayerInterface.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AYuanZuCharacterBase;
class UYuanZuAnimInstance;
class UCurveFloat;
class UYuanZuMainUIBase;
class AYuanZuGameMode;
class UYuanZuGameSettings;
class AYuanZuHUD;
class AYuanZuWeapon;

struct FInputActionValue;

UCLASS()
class YUANZU_API AYuanZuPlayerController : public APlayerController, public IMultiplayerInterface
{
	GENERATED_BODY()
public:
	AYuanZuPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	//网络复制
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//当Controller成功控制一个Pawn时调用
	virtual void OnPossess(APawn* aPawn) override;
	//当Controller不控制控制一个Pawn时调用
	virtual void OnUnPossess() override;
	//当服务器的Pawn改变时就会自动同步给客户端，然后调用通知客户端同步完成
	virtual void OnRep_Pawn() override;
	//客户端同步服务器
	virtual void ReceivedPlayer() override;

private:
	UPROPERTY()
	AYuanZuCharacterBase* YuanZuCharacter;
	UPROPERTY()
	AYuanZuHUD* YuanZuHUD;
	UPROPERTY()
	AYuanZuGameMode* YuanZuGameMode;
	/*
	* 输入----------
	*/
	//
	//映射上下文
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* YuanZuMappingContext;
	//设置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SettingsAction;
	//移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	//看
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	//跳跃
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	//跑
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* RunAction;
	//装备
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;
	//蹲下
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	//瞄准
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;
	//游泳
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SwimAction;
	//开火
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;
	//丢弃
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* DroppedAction;
	//显示局内战绩
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ShowGameRecordAction;
	//换弹
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	//游戏总时长
	float MatchTime = 0.f;
	//倒计时开始
	uint32 CountdownInt = 0;
	//客户端和服务器的差值
	float ClientServerDelta = 0.f;
	//时间同步频率
	UPROPERTY(EditAnywhere, Category = "YuanZu|Time")
	float TimeSyncFreqyency = 5.0f;
	//距离上一次同步的时间
	float TimeSyncRunningTime = 0.f;
	//热身时长
	float WarmupTime = 0.f;
	//关卡开始时间
	float LevelStartingTime = 0.f;
	//等待新对局的冷却时间
	float CooldownTime = 0.f;
	//比赛状态
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	//是否正在返回登录页面
	bool bIsReturningToLogin = false;
	//登录地图路径
	UPROPERTY(EditAnywhere, Category = "YuanZu|Login", meta = (AllowPrivateAccess = "true"))
	FString LoginPath;

	//是否已打开战绩面板
	bool bIsGameRecordOpen;
	//是否已打开设置
	bool bIsSettingOpen;
	//游戏设置
	UPROPERTY()
	UYuanZuGameSettings* GameSettings;
	//游戏设置类
	UPROPERTY(EditAnywhere, Category = "YuanZu|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UYuanZuGameSettings> GameSettingsClass;
public:
	//不允许玩家操作
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

protected:
	/*
	* 角色行为函数----------
	*/
	void ShowSetting();
	void Move(const FInputActionValue& Value);//移动
	void Look(const FInputActionValue& Value);//看
	void Jump();//跳
	void StopJumping();//停止跳
	void StartRunning();//跑
	void StopRunning();//停止跑
	void EquipButtonPressed();//装备
	void Dropped();//丢弃
	void Reload();//换弹
	void CrouchButtonPressed();//蹲下
	void AimButtonPressed();//瞄准
	void AimButtonReleased();//停止瞄准
	void Swim();//游泳
	void StopSwimming();//停止游泳
	void Fire();//开火
	void StopFire();//停止开火
	void ShowGameRecord();//显示局内战绩
	void HideGameRecord();//隐藏局内战绩
	//设置倒计时
	void SetHUDTime();
	
	//销毁会话完成返回登录界面
	UFUNCTION()
	void OnDestroySessionCompleteForReturnToLogin(bool bWasSuccessful);

	//用于获取当前服务器时间，传入获取发出的客户端时间
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeToClientRequest);
	//向客户端发送服务器的时间
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeToClientRequest, float TimeServerReceivedClientRequest);
	//检查比赛状态
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();
	//客户端在加入游戏
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName State, float Warmup, float Match, float LevelStarting, float Cooldown);

private:
	//仅在服务器上运行的函数 网络调用可靠传输
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	//丢弃武器
	UFUNCTION(Server, Reliable)
	void ServerDropped();

	UFUNCTION()
	void OnRep_MatchState();

	void CheckTimeSync(float DeltaTime);

public:
	void CloseSetting();
	//客户端时间和服务器时间同步
	virtual float GetServerTime();
	virtual void SetPlayerNameFromUI_Implementation(const FString& NewName) override;

	//本地真正开始执行：DestroySession->OpenLevel
	void BeginReturnToLogin();

	//设置比赛状态
	void OnMatchStateSet(FName State);
	void HandleInProgress();
	void HandleCooldown();
	/*
	* HUD
	*/
	void SetHealth(float Health, float NewMaxHealth);
	void SetHUDKillScore(float Score);
	void SetHUDDeathScore(int32 Score);
	void SetHUDWeaponAmmo(int32 AmmoNumber);
	void SetHUDCarriedAmmo(int32 AmmoNumber);
	void SetHUDGameTime(float GameTime);
	void SetHUDAnnouncementCountdown(float Countdown);
	void SetHUDPlayerName(FString Name);
	void SetHUDPlayerIcon(ETeamType InTeamType);
	void SetWeaponProperty(AYuanZuWeapon* Weapon);
	void ShowWeaponProperty(bool bIsVisible);

	//请求退出以登录
	UFUNCTION(BlueprintCallable, Category = YuanZu)
	void RequestQuitToLogin();
	UFUNCTION(Client, Reliable)
	void ClientReturnToLogin();
	//客户端显示武器信息
	UFUNCTION(Client, Reliable)
	void ClientShowWeaponProperty(bool bIsVisible);
	//设置玩家名称到UI
	UFUNCTION(Server, Reliable)
	void ServerSetPlayerNameFromUI(const FString& NewName);
};
