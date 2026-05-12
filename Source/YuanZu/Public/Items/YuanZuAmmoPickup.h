// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/YuanZuPickupItem.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "YuanZuAmmoPickup.generated.h"

UCLASS()
class YUANZU_API AYuanZuAmmoPickup : public AYuanZuPickupItem
{
	GENERATED_BODY()

public:
	AYuanZuAmmoPickup();
protected:
	virtual void BeginPlay()override;
public:
	virtual void Tick(float DeltaTime)override;

private:
	//子弹数量
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	int32 AmmoCount = 30;
	//子弹类型
	UPROPERTY(EditAnywhere, Category = "YuanZu|Ammo")
	EAmmoType AmmoType;

public:
	FORCEINLINE int32 GetAmmoAmmoCount()const { return AmmoCount; }
	FORCEINLINE EAmmoType GetAmmoType()const { return AmmoType; }
};
