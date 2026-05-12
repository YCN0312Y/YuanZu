// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Animation/Rests/YuanZuTurningInPlace.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "Actor/Rests/YuanZuTeamMaterial.h"
#include "Components/Rests/YuanZuCombatState.h"
#include "YuanZuCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class UYuanZuCombatComponent;
class UBoxComponent;
class UAnimMontage;
class AYuanZuWeapon;
class AYuanZuHUD;
class UDamageType;
class AController;
class AYuanZuPlayerController;
class AYuanZuPlayerState;
class UMaterialInterface;
class USphereComponent;
class USoundCue;
class USplineComponent;
class USplineMeshComponent;
class UDecalComponent;
class AYuanZuPickupItem;

struct FInputActionValue;
struct FTimerHandle;
struct FPredictProjectilePathPointData;

UCLASS()
class YUANZU_API AYuanZuCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AYuanZuCharacterBase();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	virtual void OnRep_PlayerState()override;
	//网络复制
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//组件初始化后操作
	virtual void PostInitializeComponents() override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void Jump() override;
	virtual void StopJumping() override;
protected:
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
	TObjectPtr<USpringArmComponent>CameraBoom;
	//摄像机
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|Camera")
	TObjectPtr<UCameraComponent>Camera;
	//战斗组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UYuanZuCombatComponent>Combat;
	//左手
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent>LeftHandAttack;
	//右手
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent>RightHandAttack;

	/*
	* 蒙太奇
	*/
	//开火动画蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TMap<EWeaponType, TObjectPtr<UAnimMontage>>FireWeaponMontage;
	//换弹蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TMap<EWeaponType, TObjectPtr<UAnimMontage>>ReloadMontage;
	//榴弹枪换弹
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TObjectPtr<UAnimMontage>LDQReloadMontage;
	//被子弹击中蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TObjectPtr<UAnimMontage>HitReactMontage;
	//普通攻击蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TObjectPtr<UAnimMontage>AttackMontage;
	//拉栓蒙太奇
	UPROPERTY(EditAnywhere, Category = "YuanZu|Animation")
	TObjectPtr<UAnimMontage>PullBoltMontage;

	//判断是否与武器重叠
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWidget)
	TObjectPtr<AYuanZuWeapon>OverlappingWeapon;
	//可拾取的物品
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingItem)
	TObjectPtr<AYuanZuPickupItem>OverlappingItem;
	//判断是否正在跑
	UPROPERTY(ReplicatedUsing = OnRep_IsWalking)
	bool bIsWalking;
	//判断是否正在游泳
	UPROPERTY(ReplicatedUsing = OnRep_IsSwimming)
	bool bIsSwimming;
	//玩家转向枚举
	UPROPERTY(Replicated)
	ETurningInPlace TurningInPlace;
	/*
	* 子弹
	*/
	//换弹模型
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|Ammo")
	TObjectPtr<UStaticMeshComponent>AmmoMesh;

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
	//抛掷物绘制线
	UPROPERTY(VisibleAnywhere)
	USplineComponent* ProjectileLine;

	/*
	* 摄像机
	*/
	//相机阈值
	UPROPERTY(EditAnywhere, Category = "YuanZu|Camera")
	float CameraThreshold = 200.f;
	//蹲下和站起来之间的差值
	UPROPERTY(EditAnywhere, Category = "YuanZu|Camera")
	float CrouchInterpSpeed = 10;
	//默认摄像机偏移
	FVector DefaultCameraBoomSocketOffset = FVector(0.f, 50.f, 40.f);

	//默认摄像机位置
	FVector DefaultCameraBoomRelativeLocation = FVector::ZeroVector;
	//目标摄像机位置
	float TargetCameraLocation;
	//蹲下摄像机位置
	float CrouchCameraLocation = 70;

	//蹲下时弹簧臂长度
	float CrouchArmLength = 150;
	//目标弹簧臂长度
	float TargetCrouchArmLength;
	//默认弹簧臂长度
	float DefaultTargetArmLength = 200.f;

	/*
	* 玩家状态
	*/
	//最大血量
	UPROPERTY(EditAnywhere, Category = "YuanZu|PlayerState")
	float MaxHealth = 100.f;
	//当前血量
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
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
	TMap<ETeamType, FTeamMaterial> TeamMaterial;

	/*
	* 无武器攻击
	*/
	//普通攻击名称
	TArray<FName>AttackName;
	//普通攻击伤害
	UPROPERTY(EditAnywhere, Category = "YuanZu|Attack")
	float AttackDamage = 5.f;
	//普通攻击检测距离
	UPROPERTY(EditAnywhere, Category = "YuanZu|Attack")
	float AttackTraceDistance = 75.f;
	//普通攻击检测半径
	UPROPERTY(EditAnywhere, Category = "YuanZu|Attack")
	float AttackTraceRadius = 8.f;
	//是否打中人
	bool bAttackDamageApplied = false;
	//左手变换
	FTransform LeftHandTransform;
	//右手变换
	FTransform RightHandTransform;
	//蹲下音效
	UPROPERTY(EditAnywhere, Category = "YuanZu|Sound")
	TObjectPtr<USoundCue>CrouchSound;
	//站立音效
	UPROPERTY(EditAnywhere, Category = "YuanZu|Sound")
	TObjectPtr<USoundCue>UnCrouchSound;

	/*
	* 时间轴----------
	*/
	//从走到跑的时间轴平滑更新
	FOnTimelineFloat OnRunWalkTimelineUpdateDelegate;
	//游泳的时间轴平滑更新
	FOnTimelineFloat OnSwimTimelineUpdateDelegate;
	//跑步时间轴
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|Timeline")
	TObjectPtr<UTimelineComponent>WalkingTimeline;
	//跑步缓冲时间
	UPROPERTY(EditAnywhere, Category = "YuanZu|Timeline")
	TObjectPtr<UCurveFloat>WalkRetardance;
	//游泳时间轴
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|Timeline")
	TObjectPtr<UTimelineComponent>SwimTimeline;
	//游泳缓冲时间
	UPROPERTY(EditAnywhere, Category = "YuanZu|Timeline")
	TObjectPtr<UCurveFloat>SwimRetardance;

private:
	/*
	* 网络复制回调函数----------
	*/
	//武器重叠
	UFUNCTION()
	void OnRep_OverlappingWidget(AYuanZuWeapon* LastWeapon);
	//跑步
	UFUNCTION()
	void OnRep_IsWalking();
	//游泳
	UFUNCTION()
	void OnRep_IsSwimming();
	//生命值
	UFUNCTION()
	void OnRep_CurrentHealth();
	//可拾取的物品
	UFUNCTION()
	void OnRep_OverlappingItem(AYuanZuPickupItem* LastWeapon);

	/*
	* RPC----------
	*/
	//开始跑步
	UFUNCTION(Server, Reliable)
	void ServerPlayWalkTimeline();
	//停止跑步
	UFUNCTION(Server, Reliable)
	void ServerReverseWalkTimeline();
	//开始游泳
	UFUNCTION(Server, Reliable)
	void ServerPlaySwimTimeline();
	//停止游泳
	UFUNCTION(Server, Reliable)
	void ServerReverseSwimTimeline();
	//原地旋转
	UFUNCTION(Server, Reliable)
	void ServerTurningInPlace(ETurningInPlace NewState);

	//执行徒手攻击扫射动作
	bool PerformUnarmedAttackSweep();
	//施加徒手攻击伤害
	void ApplyUnarmedAttackDamage(AActor* OtherActor);
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
	void OnRunWalkTimelineUpdate(float Alpha);
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
	//选择普通攻击蒙太奇名称
	FName SelectAttackMontageName();
	//选择开火蒙太奇名称
	FName SelectFireMontageName(bool bAimming = false);
	//选择投掷蒙太奇名称
	FName SelectThrowMontageName(bool bHighThrow);
	//选择拉栓蒙太奇名称
	FName SelectPullBoltMontageName();
	//选择换弹蒙太奇名称
	FName SelectReloadMontageName(bool bAimming = false);
	//设置蹲下和站立的摄像机位置
	void SetCameraLocation(float DeltaTime);
	//添加动画蒙太奇的名字
	void AddAttackAnimName();
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
	void SetOVerlappingItem(AYuanZuPickupItem* Item);
	bool IsWeaponEquipped();
	bool GetIsAiming();
	bool IsSwimming();
	bool GetIsFire();
	void StartWalk();
	void StopWalk();
	void StartSwim();
	void StopSwim();
	//按下开火
	void FireButtonPressed();
	//停止开火
	void FireButtonReleased();
	//播放普通攻击蒙太奇
	void PlayAttackMontage();
	//播放开火蒙太奇
	void PlayFireMontage(bool bAimming);
	//播放拉栓蒙太奇
	void PlayPullBoltMontage();
	//播放丢抛掷物蒙太奇
	void PlayAntitankFireMontage(bool bHighThrow);
	//播放换弹蒙太奇
	void PlayReloadMontage(bool bAimming);
	//停止开火蒙太奇
	void StopFireMontage();
	//角色被击中
	void PlayHitReactMontage();
	//在设置UI生命值
	void Death();
	//设置子弹类型
	void SetAmmoType(AYuanZuWeapon* InWeapon);
	//设置角色材质
	void SetCharacterMaterial(ETeamType InTeamType);
	//设置狙击枪瞄准
	void SetJJBQAim(AYuanZuWeapon* InWeapon, bool bIsAimming);

	//更新子弹
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Weapon")
	void UpdateAmmo();
	//显示子弹网格体
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Weapon")
	void ShowAmmoMesh(bool bIsVisible);
	//设置攻击碰撞
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Weapon")
	void SetAttackVaild(bool bIsVaild);

	/*
	* RPC----------
	*/
	//死亡
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();
	//受击
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHit();

	AYuanZuWeapon* GetEquippedWeapon()const;
	FVector GetHitTarget()const;
	EWeaponType GetWeaponType()const;
	bool GetIsLift()const;
	bool GetIsHighThrow()const;

	//内联函数------------------------
	FORCEINLINE UYuanZuCombatComponent* GetCombat()const { return Combat; }
	FORCEINLINE AYuanZuWeapon* GetOverlappingWeapon()const { return OverlappingWeapon; }
	FORCEINLINE AYuanZuPickupItem* GetOverlappingItem()const { return OverlappingItem; }
	FORCEINLINE UTimelineComponent* GetTimelineComp()const { return WalkingTimeline; }
	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch()const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }
	FORCEINLINE bool GetIsWalking()const { return bIsWalking; }
	FORCEINLINE UCameraComponent* GetCamera()const { return Camera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom()const { return CameraBoom; }
	FORCEINLINE float GetCurrentHealth()const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	FORCEINLINE bool GetIsDeath()const { return bIsDeath; }
	FORCEINLINE USplineComponent* GetProjectileLine()const { return ProjectileLine; }
};
