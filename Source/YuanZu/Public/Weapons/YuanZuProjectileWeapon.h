// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/YuanZuWeapon.h"
#include "YuanZuProjectileWeapon.generated.h"

class AYuanZuProjectile;

UCLASS()
class YUANZU_API AYuanZuProjectileWeapon : public AYuanZuWeapon
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AYuanZuProjectileWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "YuanZu|Projectile")
	TSubclassOf<AYuanZuProjectile> ProjectileClass;


};
