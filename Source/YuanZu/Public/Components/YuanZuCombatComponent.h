// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/YuanZuHUD.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "Animation/Rests/YuanZuCombatState.h"
#include "YuanZuCombatComponent.generated.h"
#define TRACE_LENGTH 80000;

class AYuanZuCharacterBase;
class AYuanZuWeapon;
class AYuanZuPlayerController;
class AYuanZuHUD;

struct FTimerHandle;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AYuanZuWeapon* EquippedWeapon;
	//开火
	UPROPERTY(Replicated)
	bool bFire;

private:
	UPROPERTY()
	AYuanZuCharacterBase* YuanZuCharacter;
	UPROPERTY()
	AYuanZuPlayerController* YuanZuPlayerController;
	UPROPERTY()
	AYuanZuHUD* YuanZuHUD;

	//瞄准
	UPROPERTY(Replicated)
	bool bAiming;

	//准心
	FHUDPackage HUDPackage;
	//以速度改变的准心
	float CrosshairVelocityFactor;
	//瞄准时的准心
	float CrosshairAimFactor;
	//开火时的准心
	float CrosshairShootingFactor;
	//是否可以开火
	bool bCanFire = true;
	FHitResult AimHitResult;

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

	//定时器
	FTimerHandle FireTimerHandle;

	/*
	* 弹药
	*/
	//携带的弹药
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	//携带了当前所配备武器的弹药
	TMap<EWeaponType, int32> CarriedAmmoMap;
	//突击步枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 TJBQ_StartAmmo = 30;
	//火箭筒起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 HJT_StartAmmo;
	//手枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 SQ_StartAmmo;
	//狙击步枪起始弹药
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 JJBQ_StartAmmo;
	ECombatState CombatState;

private:
	//瞄准时视场角的缩放
	void InterpFOV(float DeltaTime);
	//开始使用开火定时器
	void StartFireTimer();
	//停止使用开火定时器
	void StopFireTimer();
	//初始化携带的弹药
	void InitializeCarriedAmmo();

protected:
	//瞄准
	void SetAiming(bool bIsAiming);
	//开火
	void Fire();
	//开火
	void StartFire();

	void TraceUnderCrosshairs(FHitResult& HitResult);
	//设置准心
	void SetHUDCrosshairs(float DeltaTime);

	int32 AmountToReload();

	/*
	* RPC----------
	*/
	//瞄准
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
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

	//内联函数
	FORCEINLINE void GetAiming(bool NewAiming) { SetAiming(NewAiming); }
	FORCEINLINE FVector GetHitTarget()const { return AimHitResult.ImpactPoint; }
};
