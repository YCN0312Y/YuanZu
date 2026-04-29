// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YuanZuProjectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;

UCLASS()
class YUANZU_API AYuanZuProjectile : public AActor
{
	GENERATED_BODY()

public:
	AYuanZuProjectile();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
	
private:
	//子弹特效
	UPROPERTY(EditAnywhere, Category = "YuanZu|FX")
	UParticleSystem* Tracer;
	//子弹特效组件
	UPROPERTY(VisibleAnywhere, Category = "YuanZu|FX")
	UParticleSystemComponent* TracerComponent;
	//击中特效
	UPROPERTY(EditAnywhere, Category = "YuanZu|FX")
	UParticleSystem* ImpactParticle;
	//击中声效
	UPROPERTY(EditAnywhere, Category = "YuanZu|FX")
	USoundCue* ImpactSound;

protected:
	//子弹移动组件
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	//子弹碰撞
	UPROPERTY(EditAnywhere, Category = "YuanZu|Collision")
	UBoxComponent* CollisionBox;
	//伤害
	UPROPERTY(EditAnywhere, Category = "YuanZu|Damage")
	float Damage = 20;

protected:
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);

private:
	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
