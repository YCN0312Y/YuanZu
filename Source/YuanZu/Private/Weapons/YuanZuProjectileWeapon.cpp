// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuProjectileWeapon.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/YuanZuProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/YuanZuCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "YuanZu/Log/YuanZuLog.h"

// Sets default values
AYuanZuProjectileWeapon::AYuanZuProjectileWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AYuanZuProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AYuanZuProjectileWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYuanZuProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();//枪口位置到玩家准心位置
		FRotator TargetRotation = ToTarget.Rotation();

		if (ProjectileClass && InstigatorPawn)
		{
			//Actor生成参数
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();//拥有这个Actor的玩家
			SpawnParams.Instigator = InstigatorPawn;//生成Actor的玩家

			GetWorld()->SpawnActor<AYuanZuProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);

			UE_LOG(LogYuanZu, Warning, TEXT("Fire"));


		}
	}
}
