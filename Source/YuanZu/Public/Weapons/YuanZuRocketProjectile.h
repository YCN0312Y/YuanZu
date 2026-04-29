// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/YuanZuProjectile.h"
#include "YuanZuRocketProjectile.generated.h"

class UStaticMeshComponent;
class URocketMovementComponent;

UCLASS()
class YUANZU_API AYuanZuRocketProjectile : public AYuanZuProjectile
{
	GENERATED_BODY()
public:
	AYuanZuRocketProjectile();

public:
	//火箭弹
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMsh;
private:
	//火箭弹移动组件
	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;
	//最小伤害
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage", meta = (GameplayStatics = "true"))
	float MinDamage;
	//伤害核心区域
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage", meta = (GameplayStatics = "true"))
	float DamageCenter;	
	//伤害外围区域
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage", meta = (GameplayStatics = "true"))
	float DamagePeriphery;

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)override;
	
};
