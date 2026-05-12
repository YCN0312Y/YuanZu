// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/YuanZuHUD.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "Components/Rests/YuanZuCombatState.h"
#include "Items/Rests/YuanZuItemType.h"
#include "YuanZuCombatComponent.generated.h"
#define TRACE_LENGTH 80000;

class AYuanZuCharacterBase;
class AYuanZuWeapon;
class AYuanZuPlayerController;
class AYuanZuHUD;
class UTexture2D;
class USoundCue;
class USplineMeshComponent;
class UMaterialInterface;
class UDecalComponent;

struct FTimerHandle;
struct FPredictProjectilePathPointData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YUANZU_API UYuanZuCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UYuanZuCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//网络复制
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	friend AYuanZuCharacterBase;
	//已装备武器
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EquippedWeapon)
	AYuanZuWeapon* EquippedWeapon;
	//开火
	UPROPERTY(Replicated)
	bool bFire;
	//准心
	FHUDPackage HUDPackage;
	//设置是低抛还是高抛
	bool bIsHighThrow = true;
	//举起手雷
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "YuanZu|Antitank")
	bool bIsLift = false;
	//手雷是否已扔出
	bool bIsThrow = false;
	//是否可以扔出
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "YuanZu|Antitank")
	bool bCanThrow = true;
	//手榴弹起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 SLD_StartAmmo = 2;

private:
	UPROPERTY()
	AYuanZuCharacterBase* YuanZuCharacter;
	UPROPERTY()
	AYuanZuPlayerController* YuanZuPlayerController;
	UPROPERTY()
	AYuanZuHUD* YuanZuHUD;

	//准心
	UPROPERTY(EditAnywhere, Category = "YuanZu|HUD")
	UTexture2D* CrosshairCenter;
	//瞄准声效
	UPROPERTY(EditAnywhere, Category = "YuanZu|Aim")
	TArray<USoundCue*>AimSound;
	//取消瞄准
	UPROPERTY(EditAnywhere, Category = "YuanZu|Aim")
	USoundCue* UnAimSound;
	//瞄准
	UPROPERTY(Replicated)
	bool bAiming;
	//以速度改变的准心
	float CrosshairVelocityFactor;
	//瞄准时的准心
	float CrosshairAimFactor;
	//开火时的准心
	float CrosshairShootingFactor;
	//是否可以开火
	bool bCanFire = true;
	//瞄准射线结果
	FHitResult AimHitResult;
	//定时器
	FTimerHandle FireTimerHandle;

	/*
	* 视场角----------
	*/
	//默认视场角
	float DefaultFOV;
	//当前视场角
	float CurrentFOV;
	//瞄准视场角
	UPROPERTY(EditAnywhere, Category = "YuanZu|FOV")
	float AimFOV = 30.f;
	//瞄准插值速度
	UPROPERTY(EditAnywhere, Category = "YuanZu|FOV")
	float AimInterpSpeed = 20.f;

	/*
	* 武器
	*/
	EWeaponSlot WeaponSlot;
	//武器插槽的武器
	TArray<TObjectPtr<AYuanZuWeapon>> ListWeapon;


	/*
	* 弹药
	*/
	//携带的弹药
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	//携带了当前所配备武器的弹药
	TMap<EAmmoType, int32> CarriedAmmoMap;
	//自动步枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 ZDBQ_StartAmmo = 30;
	//突击步枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 TJBQ_StartAmmo = 30;
	//火箭筒起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 HJT_StartAmmo = 1;
	//手枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 SQ_StartAmmo = 15;
	//狙击步枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 JJBQ_StartAmmo = 10;
	//榴弹枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 LDQ_StartAmmo = 5;
	/*
	* 抛掷物
	*/
	//绘制的网格 
	TArray<USplineMeshComponent*>LineMeshes;
	//抛物点模型
	UPROPERTY(EditAnywhere, Category = "YuanZu|Projectile")
	UStaticMesh* ParabolicLineMesh;
	//抛物线模型材质
	UPROPERTY(EditAnywhere, Category = "YuanZu|Projectile")
	TObjectPtr<UMaterialInterface> LineMeshesMaterial;
	//抛物点模型材质
	UPROPERTY(EditAnywhere, Category = "YuanZu|Projectile")
	TObjectPtr<UMaterialInterface> ParabolicPointMaterial;
	//瞄准点
	UPROPERTY()
	TObjectPtr<UDecalComponent>ParabolicPointDecal;

private:
	//瞄准时视场角的缩放
	void InterpFOV(float DeltaTime);
	//开始使用开火定时器
	void StartFireTimer();
	//停止使用开火定时器
	void StopFireTimer();
	//初始化携带的弹药
	void InitializeCarriedAmmo();
	//开始攻击
	void StartAttack();
	//绘制手雷路径
	void DrawAntitankPath();
	//更新抛物线
	void UpdateProjectileLine(const TArray<FPredictProjectilePathPointData>& PathData, const FHitResult& HitResult);
	//清除抛物线
	void ClearProjectileLine();
	//是否是本地玩家
	bool IsLocalPlayer();
	//处理手雷投掷后的武器状态
	void HandleAntitankState();
	//添加携带的子弹
	void AddCarriedAmmo(EAmmoType InAmmoType, int32 InAmmoCount);
	//设置武器插槽
	void SetWeaponSlot(AYuanZuWeapon* InWeapon);
protected:
	//瞄准
	void SetAiming(bool bIsAiming);
	//开火
	void Fire();
	//开火
	void StartFire();
	//准心位置
	void TraceUnderCrosshairs(FHitResult& HitResult);
	//设置准心
	void SetHUDCrosshairs(float DeltaTime);
	//重新装填量
	int32 AmountToReload();
	//开始举起投掷物
	void StartLift();
	//取消举起投掷物
	void CencelLift();
	//停止举起投掷物
	void StopLift();

	/*
	* RPC----------
	*/
	//普通攻击
	UFUNCTION(Server, Reliable)
	void ServerAttack();
	//多播普通攻击
	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttack();
	//瞄准
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	//多播瞄准
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetAiming(bool bIsAiming);
	//开火
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitTarget);
	//多播开火
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& HitTarget);
	//停止开火
	UFUNCTION(Server, Reliable)
	void ServerStopFire();
	//多播停止开火
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopFire();
	//换弹
	UFUNCTION(Server, Reliable)
	void ServerReload();
	//换弹多播
	UFUNCTION(NetMulticast, Reliable)
	void MulticastReload();
	//打开投掷物拉栓
	UFUNCTION(Server, Reliable)
	void ServerPullBolt();
	//打开投掷物拉栓多播
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPullBolt();
	//投掷物
	UFUNCTION(Server, Reliable)
	void ServerAntitank(bool bHighThrow);
	//投掷物多播
	UFUNCTION(NetMulticast, Reliable)
	void MulticastAntitank(bool bHighThrow);
	//炸弹投出
	UFUNCTION(BlueprintCallable ,Server, Reliable, Category = "YuanZu|Combat")
	void ServerFireAntitank();
	//投掷动画结束后同步手雷状态
	UFUNCTION(Server, Reliable)
	void ServerSetAntitankState();
	//网络复制
	UFUNCTION()
	void OnRep_EquippedWeapon();//当客户端玩家装备武器后调用该函数
	//携带的弹药
	UFUNCTION()
	void OnRep_CarriedAmmo();

public:
	//停止开火
	void StopFire();
	//装备武器
	void EquipWeapon(AYuanZuWeapon* WeaponToEquip);
	//丢弃武器
	void DroppedWeapon();
	//换弹
	void Reload();
	//是否可以开火
	bool CanFire();
	//更新子弹数量
	void UpdateAmmoCount();
	//消耗指定类型的携带弹药
	bool ConsumeCarriedAmmo(EAmmoType AmmoType, int32 AmmoAmount);
	//获取指定类型的携带弹药数量
	int32 GetCarriedAmmoByType(EAmmoType AmmoType) const;
	//设置是否已投出
	void SetThrow(bool bIsThrow);
	//生成投掷物
	void SpawnAntitank();
	//停止计时
	void CencelDetonation();
	//设置高抛还是低抛
	void SetThrowMode(bool& bHeight);
	//拾取弹药
	void PickupAmmo(EAmmoType InAmmoType, int32 InAmmoCount);


	//投掷物丢出
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Combat")
	void FireAntitank();
	//设置投掷物为空
	UFUNCTION(BlueprintCallable, Category = "YuanZu|Combat")
	void SetAntitankState();
	//获取抛掷物开始位置
	FVector GetAntitankStartLocation()const;
	//获取抛掷物开始速度
	FVector GetAntitankLaunchVelocity()const;

	//内联函数
	FORCEINLINE void GetAiming(bool NewAiming) { SetAiming(NewAiming); }
	FORCEINLINE FVector GetHitTarget()const { return AimHitResult.ImpactPoint; }
	FORCEINLINE TMap<EAmmoType, int32> GetAmmoType()const { return CarriedAmmoMap; }
	FORCEINLINE bool GetIsThrow()const { return bIsThrow; }
	FORCEINLINE bool GetIsLift()const { return bIsLift; }
	FORCEINLINE bool GetIsHighThrow()const { return bIsHighThrow; }
	FORCEINLINE int32 GetCarriedAmmo() { return CarriedAmmo; }
};
