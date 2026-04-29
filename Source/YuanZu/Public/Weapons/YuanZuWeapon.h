// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "YuanZuWeapon.generated.h"

class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class AYuanZuBulletShell;
class UTexture2D;
class AYuanZuCharacterBase;
class AYuanZuPlayerController;
class USoundCue;
class UYuanZuPickUp;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

struct FYuanZuAmmoType;

UENUM()
enum class EWeaponState : uint8
{
	//初始化武器 并未捡起时
	EWS_Initial UMETA(Display = "Initial State"),
	//武器被捡起时
	EWS_Equipped UMETA(Display = "Equipped"),
	//武器被丢弃
	EWS_Dropped UMETA(Display = "Dropped"),
	//默认最大常量
	EWS_Max UMETA(Display = "DefaultMAX")
};

UCLASS()
class YUANZU_API AYuanZuWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AYuanZuWeapon();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Owner() override;
	//显示子弹网格体
	UFUNCTION(BlueprintCallable, Category = YuanZu)
	virtual void ShowAmmoMesh(bool bIsVisible);

private:
	UPROPERTY()
	AYuanZuCharacterBase* YuanZuOwnerCharacter;
	UPROPERTY()
	AYuanZuPlayerController* YuanZuOwnerPlayerController;

	//武器骨骼网格体
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* WeaponMesh;
	//子弹网格体
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AmmoMesh;
	//球体碰撞
	UPROPERTY(VisibleAnywhere)
	USphereComponent* AreaSphere;
	//武器状态
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "YuanZu|PickUp")
	EWeaponState WeaponState;
	//拾取小组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YuanZu|PickUp", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickUpWidget;
	//拾取小组件类
	UPROPERTY(EditAnywhere, Category = "YuanZu|PickUp");
	TSubclassOf<UYuanZuPickUp>PickUp;
	//开火动画
	UPROPERTY(EditAnywhere, Category = "YuanZu|Fire")
	UAnimationAsset* FireAnimation;
	UPROPERTY(EditAnywhere, Category = "YuanZu|Fire")
	float ShootingSpeed = 40.f;
	//子弹壳类
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	TSubclassOf<AYuanZuBulletShell>BulletShellClass;
	//瞄准的视场角
	UPROPERTY(EditAnywhere, Category = "YuanZu|FOV")
	float AimFOV = 30.f;
	//瞄准的插值速度
	UPROPERTY(EditAnywhere, Category = "YuanZu|FOV")
	float AimInterpSpeed = 20.f;
	//武器类型
	UPROPERTY(EditAnywhere, Category = "YuanZu|Weapon")
	EWeaponType WeaponType;
	/*
	* 弹药
	*/
	//当前弹药数量
	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;
	//弹夹最大容量
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 MagCapacity;
	//默认弹夹材质
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	UMaterialInterface* DefaultMaterial;
	//换弹弹夹材质
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	UMaterialInterface* ReloadMaterial;
public:
	/*
	* 瞄准准心----------
	*/
	//中
	UPROPERTY(EditAnywhere, Category = "YuanZu|HUD")
	UTexture2D* CrosshairCenter;
	//左
	UPROPERTY(EditAnywhere, Category = "YuanZu|HUD")
	UTexture2D* CrosshairLeft;
	//右
	UPROPERTY(EditAnywhere, Category = "YuanZu|HUD")
	UTexture2D* CrosshairRight;
	//上
	UPROPERTY(EditAnywhere, Category = "YuanZu|HUD")
	UTexture2D* CrosshairTop;
	//下
	UPROPERTY(EditAnywhere, Category = "YuanZu|HUD")
	UTexture2D* CrosshairBottom;

	/*
	* 自动射击
	*/
	//开火延迟
	UPROPERTY(EditAnywhere, Category = "YuanZu|Fire", meta = (ToolTip = "The time interval for firing bullets"))
	float FireDelay = 0.15f;
	//自动开火
	UPROPERTY(EditAnywhere, Category = "YuanZu|Fire")
	bool bAutoFire = true;
	//拾取武器声效
	UPROPERTY(EditAnywhere, Category = "YuanZu|PickUp")
	USoundCue* PickUpSound;
	//武器名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YuanZu|PickUp")
	FText WeaponName;
	//子弹类型
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	UStaticMesh* AmmoTypeMesh;

private:
	//与武器开始重叠
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//与武器结束重叠
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//子弹消耗
	void SpendRound();
	//设置武器材质
	void SetWeaponMaterial(bool bIsVisible);
	/*
	* 网络复制
	*/
	//武器状态
	UFUNCTION()
	void OnRep_WeaponState();
	//弹药
	UFUNCTION()
	void OnRep_Ammo();

public:
	//开火
	virtual void Fire(const FVector& HitTarget);
	//设置武器状态
	void SetWeaponState(EWeaponState State);
	//显示拾取控件
	void ShowPickUpWidget(bool bShowWidget);
	//丢弃武器
	void DroppedWeapon();
	//弹夹子弹是否为空
	bool IsEmpty();
	//设置弹药UI
	void SetHUDAmmo();
	//添加弹药
	void AddAmmo(int32 AmmoToAdd);

	UFUNCTION(BlueprintPure, Category = YuanZu)
	FText GetWeaponDisplayName() const { return WeaponName; }

	//内联函数----------
	FORCEINLINE USphereComponent* GetAreaSphere()const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh()const { return WeaponMesh; }
	FORCEINLINE float GetAimFOV()const { return AimFOV; }
	FORCEINLINE float GetAimInterpSpeed()const { return AimInterpSpeed; }
	FORCEINLINE float GetShootingSpeed()const { return ShootingSpeed; }
	FORCEINLINE EWeaponState GetWeaponState()const { return WeaponState; }
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; }
	FORCEINLINE int32 GetAmmo()const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity()const { return MagCapacity; }
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh()const { return AmmoMesh; }

};
