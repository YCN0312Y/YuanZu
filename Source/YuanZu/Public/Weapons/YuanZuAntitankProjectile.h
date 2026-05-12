// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/YuanZuProjectile.h"
#include "YuanZuAntitankProjectile.generated.h"

class UStaticMeshComponent;
class USoundCue;

struct FTimerHandle;

UCLASS()
class YUANZU_API AYuanZuAntitankProjectile : public AYuanZuProjectile
{
	GENERATED_BODY()

public:
	AYuanZuAntitankProjectile();

protected:
	virtual void BeginPlay()override;
private:
	UPROPERTY(EditAnywhere,Category = "YuanZu|Ammo")
	TObjectPtr<UStaticMeshComponent>ProjectileMesh;
	//爆炸计时器句柄
	FTimerHandle DetonationTimer;
	//命中后开启物理效果
	bool bPhysicsEnabledAfterHit = false;
	//最小伤害
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage")
	float MinDamage;
	//伤害内半径
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage")
	float DamageCenter;
	//伤害外半径
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage")
	float DamagePeriphery;
	//落地声音
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage")
	TObjectPtr<USoundCue>FloorSound;

public:
	//设置发射速度
	void SetLaunchVelocity(const FVector& InVelocity);
	//投掷物爆炸
	void AntitankDestory();
	//设置抛掷物碰撞
	void SetProjectileMeshCollision(bool bThrow);
	//开始倒计时
	void StartFuse(float FuseTime);
	//取消倒计时
	void CancelFuse();
	//播放落地声音
	UFUNCTION()
	void PlayFloorSound(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
