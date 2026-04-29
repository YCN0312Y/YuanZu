// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Animation/Rests/YuanZuTurningInPlace.h"
#include "Interfaces/YuanZuInterctWithCrosshairsInterface.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class UYuanZuCombatComponent;
class USoundBase;
class UBoxComponent;
class UAnimMontage;
class AYuanZuWeapon;
class AYuanZuHUD;
class UDamageType;
class AController;
class AYuanZuPlayerController;
class AYuanZuPlayerState;
class UMaterialInterface;

struct FInputActionValue;
struct FTimerHandle;

UCLASS()
class YUANZU_API AYuanZuCharacterBase : public ACharacter, public IYuanZuInterctWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AYuanZuCharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
	virtual void OnRep_PlayerState()override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//网络复制
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//组件初始化后操作
	virtual void PostInitializeComponents() override;

	virtual void Crouch(bool bClientSimulation = false)override;
	virtual void UnCrouch(bool bClientSimulation = false)override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//换弹
	UPROPERTY(BlueprintReadWrite, Category = "YuanZu|Weapon")
	bool bIsReload;

private:
	UPROPERTY()
	AYuanZuPlayerController* YuanZuPlayerController;
	UPROPERTY()
	AYuanZuHUD* YuanZuHUD;
	UPROPERTY()
	AYuanZuPlayerState* YuanZuPlayerState;
	/*
	* 组件----------
	*/
	//弹簧臂
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|Camera")
	USpringArmComponent* CameraBoom;
	//摄像机
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|Camera")
	UCameraComponent* Camera;
	//头顶小部件
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YuanZu|Widget", meta = (AllowPrivateAccess = "true"))
	//UWidgetComponent* OverHeadWidget;//头顶小部件
	//战斗组件
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UYuanZuCombatComponent* Combat;

	/*
	* 蒙太奇
	*/
	//开火动画蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TArray<UAnimMontage*> FireWeaponMontage;
	//换弹蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TArray<UAnimMontage*> ReloadMontage;
	//被子弹击中蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	UAnimMontage* HitReactMontage;
	//没有武器被子弹击中蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	UAnimMontage* HitReactMontage_NoWeapon;

	/*
	* 网络复制变量----------
	*/
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWidget)
	AYuanZuWeapon* OverlappingWeapon;//判断是否与武器重叠
	UPROPERTY(ReplicatedUsing = OnRep_IsRunning)
	bool bIsRunning;//判断是否正在跑
	UPROPERTY(ReplicatedUsing = OnRep_IsSwimming)
	bool bIsSwimming;//判断是否正在游泳
	UPROPERTY(Replicated)
	ETurningInPlace TurningInPlace;//玩家转向枚举

	/*
	* 瞄准偏移----------
	*/
	//左右
	float AO_Yaw;
	//左右插值
	float InterpAO_Yaw;
	//上下
	float AO_Pitch;
	//起始瞄准旋转
	FRotator StartAimRotation;
	//连续Yaw
	float ContinuousYaw = 0.f;
	//上一个Yaw
	float LastYaw = 0.f;
	//视角Yaw
	float ViewYaw;

	/*
	* 蓝图使用的变量----------
	*/
	//相机阈值
	UPROPERTY(EditAnywhere, Category = "YuanZu|Camera")
	float CameraThreshold = 200.f;
	//蹲下和站起来之间的差值
	UPROPERTY(EditAnywhere, Category = "YuanZu|Camera")
	float CrouchInterpSpeed;
	//目标设计相机位置
	float TargetCameraLocation;
	//蹲下摄像机位置
	UPROPERTY(EditAnywhere, Category = "YuanZu|Camera")
	float CrouchCameraLocation;
	//站立摄像机位置
	UPROPERTY(EditAnywhere, Category = "YuanZu|Camera")
	float StandCameraLocation;

	/*
	* 玩家状态
	*/
	//最大血量
	UPROPERTY(EditAnywhere, Category = "YuanZu|PlayerState")
	float MaxHealth = 100.f;
	//当前血量
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, Category = "YuanZu|PlayerState")
	float CurrentHealth = 100.f;
	//是否已经死亡
	UPROPERTY(Replicated)
	bool bIsDeath = false;
	//复活定时器
	FTimerHandle RebirthTimerHandle;
	//复活等待时间
	UPROPERTY(EditAnywhere, Category = "YuanZu|PlayerState")
	float RebirthDelay = 3.f;
	//角色材质
	UPROPERTY(EditAnywhere, Category = "YuanZu|PlayerState")
	TArray<UMaterialInterface*> TeamCharacterMaterial;

	/*
	* 时间轴绑定代理----------
	*/
	FOnTimelineFloat OnWalkRunTimelineUpdateDelegate;//从走到跑的时间轴平滑更新
	FOnTimelineFloat OnSwimTimelineUpdateDelegate;//游泳的时间轴平滑更新

	/*
	* 子弹
	*/
	//子弹模型
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|Ammo")
	UStaticMeshComponent* AmmoMesh;

protected:
	/*
	* 时间轴----------
	*/
	//跑步时间轴
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "YuanZu|Timeline")
	UTimelineComponent* RunningTimeline;
	//跑步缓冲时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Timeline")
	UCurveFloat* RunningRetardance;
	//游泳时间轴
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "YuanZu|Timeline")
	UTimelineComponent* SwimTimeline;
	//游泳缓冲时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YuanZu|Timeline")
	UCurveFloat* SwimRetardance;

private:
	/*
	* 网络复制回调函数----------
	*/
	//武器重叠
	UFUNCTION()
	void OnRep_OverlappingWidget(AYuanZuWeapon* LastWeapon);
	//跑步
	UFUNCTION()
	void OnRep_IsRunning();
	//游泳
	UFUNCTION()
	void OnRep_IsSwimming();
	//生命值
	UFUNCTION()
	void OnRep_CurrentHealth();

	/*
	* RPC----------
	*/
	//开始跑步
	UFUNCTION(Server, Reliable)
	void ServerPlayRunTimeline();
	//停止跑步
	UFUNCTION(Server, Reliable)
	void ServerReverseRunTimeline();
	//开始游泳
	UFUNCTION(Server, Reliable)
	void ServerPlaySwimTimeline();
	//停止游泳
	UFUNCTION(Server, Reliable)
	void ServerReverseSwimTimeline();
	//原地旋转
	UFUNCTION(Server, Reliable)
	void ServerTurningInPlace(ETurningInPlace NewState);

	//获取角色在不移动的情况下的摄像机旋转角度
	FRotator GetAimRotation()const;
	//角色旋转角度
	void TurnInPlace(float DeltaTime);
	//进入游泳状态
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Swim")
	void SwimState();
	//退出游泳状态
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Swim")
	void EndSwimState();
	//跑步更新时间轴
	UFUNCTION()
	void OnWalkRunTimelineUpdate(float Alpha);
	//游泳更新时间轴
	UFUNCTION()
	void OnSwimTimelineUpdate(float Alpha);
	//设置伤害
	UFUNCTION()
	void SetDemage(AActor* DemageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	//重生
	void Rebirth();
	//战绩初始化
	void PollInit();
	//选择开火蒙太奇名称
	FName SelectFireMontageName(bool bAimming = false);
	//选择换弹蒙太奇名称
	FName SelectReloadMontageName(bool bAimming = false);
	//设置蹲下和站立的摄像机位置
	void SetCameraLocation(float DeltaTime);
protected:
	//瞄准偏移
	void AimOffset(float DeltaTime);
	//更新生命值
	void UpdateHealth();

public:
	/*
	* 逻辑函数----------
	*/
	void Move(const FInputActionValue& Value);//移动
	void Look(const FInputActionValue& Value);//看
	void SetOverlappingWeapon(AYuanZuWeapon* Weapon);
	bool IsWeaponEquipped();
	bool GetIsAiming();
	bool IsSwimming();
	bool GetIsFire();
	void StartRun();
	void StopRun();
	void StartSwim();
	void StopSwim();
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayFireMontage(bool bAimming);
	void PlayReloadMontage();
	void StopFireMontage();
	//角色被击中
	void PlayHitReactMontage(bool bIsHit);
	//在设置UI生命值
	void Death();
	//设置子弹类型
	void SetAmmoType(AYuanZuWeapon* InWeapon);
	//设置角色材质
	void SetCharacterMaterila(ETeamType InTeamType);

	//更新子弹
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Weapon")
	void UpdateAmmo();
	//显示子弹网格体
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Weapon")
	void ShowAmmoMesh(bool bIsVisible);

	/*
	* RPC----------
	*/
	//死亡
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();

	AYuanZuWeapon* GetEquippedWeapon()const;
	FVector GetHitTarget()const;
	EWeaponType GetWeaponType()const;
	//内联函数------------------------
	FORCEINLINE UYuanZuCombatComponent* GetCombat()const { return Combat; }
	FORCEINLINE AYuanZuWeapon* GetOverlappingWeapon()const { return OverlappingWeapon; }
	FORCEINLINE UTimelineComponent* GetTimelineComp()const { return RunningTimeline; }
	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch()const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }
	FORCEINLINE bool GetIsRunning()const { return bIsRunning; }
	FORCEINLINE UCameraComponent* GetCamera()const { return Camera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom()const { return CameraBoom; }
	FORCEINLINE float GetCurrentHealth()const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	FORCEINLINE bool GetIsDeath()const { return bIsDeath; }
};
