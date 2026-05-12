// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuAntitankProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AYuanZuAntitankProjectile::AYuanZuAntitankProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	//子弹运动组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 0.f;
	ProjectileMovementComponent->MaxSpeed = 3000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;//旋转遵循速度
	ProjectileMovementComponent->ProjectileGravityScale = 1.f;//弹丸重力
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;//使用本地空间的速度
	ProjectileMovementComponent->bAutoActivate = false;

}

void AYuanZuAntitankProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionBox)
	{
		CollisionBox->OnComponentHit.RemoveAll(this);
		CollisionBox->OnComponentHit.AddDynamic(this, &AYuanZuAntitankProjectile::PlayFloorSound);
	}
}

void AYuanZuAntitankProjectile::SetLaunchVelocity(const FVector& InVelocity)
{
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->Velocity = InVelocity;
		ProjectileMovementComponent->Activate();
	}
}

void AYuanZuAntitankProjectile::AntitankDestory()
{
	if (!HasAuthority()) return;

	Destroy();

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
}

void AYuanZuAntitankProjectile::SetProjectileMeshCollision(bool bThrow)
{
	if (bThrow)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
		CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		ProjectileMovementComponent->bShouldBounce = true;
		ProjectileMovementComponent->Bounciness = 0.6f;
		ProjectileMovementComponent->Friction = 0.3f;
		ProjectileMovementComponent->BounceVelocityStopSimulatingThreshold = 80.f;
	}
	else
	{
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetSimulatePhysics(false);
		CollisionBox->SetEnableGravity(false);

	}
}

void AYuanZuAntitankProjectile::StartFuse(float FuseTime)
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().SetTimer(DetonationTimer, this, &AYuanZuAntitankProjectile::AntitankDestory, FuseTime);
	int32 DetonationFuseTime = FMath::Floor(FuseTime);
	UE_LOG(LogYuanZu, Warning, TEXT("手雷爆炸倒计时[%d]"), DetonationFuseTime);
}

void AYuanZuAntitankProjectile::CancelFuse()
{
	if (!HasAuthority()) return;
	GetWorldTimerManager().ClearTimer(DetonationTimer);
	UE_LOG(LogYuanZu, Warning, TEXT("已取消手雷"));
}

void AYuanZuAntitankProjectile::PlayFloorSound(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (FloorSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FloorSound, GetActorLocation());
	}
}
