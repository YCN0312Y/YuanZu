// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuRocketProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/RocketMovementComponent.h"

AYuanZuRocketProjectile::AYuanZuRocketProjectile()
{
	RocketMsh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMsh->SetupAttachment(RootComponent);
	RocketMsh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;//旋转随速度而发生
	RocketMovementComponent->SetIsReplicated(true);
	RocketMovementComponent->InitialSpeed = 3000.f;
	RocketMovementComponent->MaxSpeed = 3000.f;
}

void AYuanZuRocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	//获取发起者
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			//伤害衰减，里火箭弹越近的伤害越高，越远的伤害越低
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,//当前世界环境
				Damage,//基础伤害
				MinDamage,//最小伤害
				GetActorLocation(),//伤害半径的中心位置
				DamageCenter,//伤害内半径
				DamagePeriphery,//伤害外半径
				1.f,//伤害值衰减
				UDamageType::StaticClass(),//伤害类型
				TArray<AActor*>(),//不受到伤害的实体
				this,//伤害发起者
				FiringController//攻击者控制器
			);
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
}
