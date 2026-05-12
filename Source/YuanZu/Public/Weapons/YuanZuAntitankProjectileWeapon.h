// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/YuanZuProjectileWeapon.h"
#include "YuanZuAntitankProjectileWeapon.generated.h"

class AYuanZuProjectile;

UCLASS()
class YUANZU_API AYuanZuAntitankProjectileWeapon : public AYuanZuProjectileWeapon
{
	GENERATED_BODY()

public:
	virtual void SpendRound()override;

public:
	UPROPERTY()
	TObjectPtr<AYuanZuProjectile>Projectile;
	UPROPERTY(EditAnywhere, Category = "YuanZu|Antitank")
	float DetonationTime = 5.f;
public:
	//丢出手雷
	void FireAntitank(const FVector& LaunchVelocity);
	//生成手雷
	void SpawnAntitank(const FVector& StartLocation, const FVector& LaunchVelocity);
	//取消爆炸
	void CencelDetonation();
	//装填手雷
	void ReloadAmmo();
	
};
