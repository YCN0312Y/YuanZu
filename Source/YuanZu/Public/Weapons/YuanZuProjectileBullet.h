// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/YuanZuProjectile.h"
#include "YuanZuProjectileBullet.generated.h"

UCLASS()
class YUANZU_API AYuanZuProjectileBullet : public AYuanZuProjectile
{
	GENERATED_BODY()

public:
	AYuanZuProjectileBullet();
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;

};
