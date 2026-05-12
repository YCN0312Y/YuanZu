// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuAntitankProjectileWeapon.h"
#include "Weapons/YuanZuAntitankProjectile.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/YuanZuCombatComponent.h"
#include "Character/YuanZuCharacterBase.h"

void AYuanZuAntitankProjectileWeapon::SpendRound()
{
	Super::SpendRound();
}

void AYuanZuAntitankProjectileWeapon::FireAntitank(const FVector& LaunchVelocity)
{
	if (!Projectile)return;
	UE_LOG(LogYuanZu, Warning, TEXT("FireAntitank"));

	AYuanZuAntitankProjectile* AntitankProjectile = Cast<AYuanZuAntitankProjectile>(Projectile);
	if (!AntitankProjectile) return;

	AntitankProjectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	AntitankProjectile->SetProjectileMeshCollision(true);
	AntitankProjectile->SetLaunchVelocity(LaunchVelocity);
	SpendRound();
	ReloadAmmo();
	Projectile = nullptr;
}

void AYuanZuAntitankProjectileWeapon::SpawnAntitank(const FVector& StartLocation, const FVector& LaunchVelocity)
{
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (!ProjectileClass || !InstigatorPawn || !GetWorld()) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = InstigatorPawn;

	Projectile = GetWorld()->SpawnActor<AYuanZuAntitankProjectile>(
		ProjectileClass,
		StartLocation,
		LaunchVelocity.Rotation(),
		SpawnParams
	);

	if (Projectile)
	{
		Projectile->AttachToComponent(
			InstigatorPawn->FindComponentByClass<USkeletalMeshComponent>(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName("RightHandSocket")
		);
		AYuanZuAntitankProjectile* AntitankProjectile = Cast<AYuanZuAntitankProjectile>(Projectile);
		if (AntitankProjectile)
		{
			AntitankProjectile->SetProjectileMeshCollision(false);
			AntitankProjectile->StartFuse(DetonationTime);
		}
	}
	
}

void AYuanZuAntitankProjectileWeapon::CencelDetonation()
{
	if (!Projectile)return;
	UE_LOG(LogYuanZu, Warning, TEXT("FireAntitank"));

	AYuanZuAntitankProjectile* AntitankProjectile = Cast<AYuanZuAntitankProjectile>(Projectile);
	if (!AntitankProjectile) return;

	AntitankProjectile->CancelFuse();
}

void AYuanZuAntitankProjectileWeapon::ReloadAmmo()
{
	if (Ammo >= MagCapacity) return;

	AYuanZuCharacterBase* YZC = Cast<AYuanZuCharacterBase>(GetOwner());
	if (YZC && YZC->GetCombat() && YZC->GetCombat()->ConsumeCarriedAmmo(AmmoType, 1))
	{
		Ammo = FMath::Clamp(Ammo + 1, 0, MagCapacity);
		SetHUDAmmo();
	}
}
