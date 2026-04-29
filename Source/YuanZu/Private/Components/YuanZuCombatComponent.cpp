// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/YuanZuCombatComponent.h"
#include "Character/YuanZuCharacterBase.h"
#include "Weapons/YuanZuWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Gameplay/YuanZuPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "UI/YuanZuMainUIBase.h"

UYuanZuCombatComponent::UYuanZuCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UYuanZuCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (YuanZuCharacter)
	{
		if (YuanZuCharacter->GetCamera())
		{
			DefaultFOV = YuanZuCharacter->GetCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (YuanZuCharacter->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}

}

void UYuanZuCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (YuanZuCharacter && YuanZuCharacter->IsLocallyControlled())
	{
		TraceUnderCrosshairs(AimHitResult);
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

}

void UYuanZuCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UYuanZuCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UYuanZuCombatComponent, bAiming);
	DOREPLIFETIME(UYuanZuCombatComponent, bFire); 
	DOREPLIFETIME_CONDITION(UYuanZuCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UYuanZuCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon)
	{
		if (bAiming)
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetAimFOV(), DeltaTime, EquippedWeapon->GetAimInterpSpeed());
		}
		else
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, AimInterpSpeed);
		}
		if (YuanZuCharacter && YuanZuCharacter->GetCamera())
		{
			YuanZuCharacter->GetCamera()->SetFieldOfView(CurrentFOV);
		}
	}

}

void UYuanZuCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2D ViewportSize;//游戏视口大小

	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);//瞄准点位置
	FVector CrosshairWorldPosition;//瞄准点世界位置
	FVector CrosshairWorldDirection;//瞄准点世界方向
		
	//投影屏幕至世界坐标系
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(YuanZuPlayerController, CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (YuanZuCharacter)
		{
			float DistanceToCharacter = (YuanZuCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100);
			//DrawDebugPoint(GetWorld(), Start, 10.f, FColor::Red);
		}
		FVector End = Start + CrosshairWorldDirection * 80000.f;

		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

		if (HitResult.bBlockingHit && HitResult.GetActor() && HitResult.GetActor()->Implements<UYuanZuInterctWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red);
			//DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Red);

		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
			//DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Green);

		}
		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}

	}
}

void UYuanZuCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (YuanZuCharacter == nullptr || YuanZuCharacter->Controller == nullptr) return;

	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	if (YuanZuPlayerController)
	{
		YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(YuanZuPlayerController->GetHUD()) : YuanZuHUD;
		if (YuanZuHUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{

				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//计算准心扩展
			FVector2D RunningSpeed(0.f, YuanZuCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplier(0.f, 1.f);//速度乘数
			FVector Velocity = YuanZuCharacter->GetVelocity();
			Velocity.Z = 0.f;
			float Speed = Velocity.Size();

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(RunningSpeed, VelocityMultiplier, Speed);
			if (EquippedWeapon)
			{
				if (bAiming)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, EquippedWeapon->GetAimInterpSpeed());
				}
				else
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, EquippedWeapon->GetAimInterpSpeed());
				}
				CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, EquippedWeapon->GetShootingSpeed());
			}

			HUDPackage.CrosshairSpread = 0.5 + CrosshairVelocityFactor - CrosshairAimFactor + CrosshairShootingFactor;

			YuanZuHUD->SetHUDPackage(HUDPackage);

		}
	}
}

int32 UYuanZuCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	//弹夹可装填的弹药数量（弹夹最大容量 - 剩余子弹数量）
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//背包携带的弹药量
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		//如果背包剩余的数量少就都给弹夹。如果弹夹数量小，那就补弹夹可以补的那部分
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UYuanZuCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;

	ServerSetAiming(bIsAiming);

}

void UYuanZuCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UYuanZuCombatComponent::EquipWeapon(AYuanZuWeapon* WeaponToEquip)
{
	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;

	if (!YuanZuCharacter || !WeaponToEquip || !YuanZuPlayerController) return;

	if (EquippedWeapon)
	{
		//丢弃武器
		EquippedWeapon->DroppedWeapon();
	}
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	YuanZuCharacter->SetAmmoType(EquippedWeapon);
	const USkeletalMeshSocket* HandSocket = YuanZuCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, YuanZuCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(YuanZuCharacter);
	EquippedWeapon->SetHUDAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//将武器类型的子弹数量放到背包剩余容量
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	if (YuanZuPlayerController)
	{	
		//显示当前武器属性UI
		YuanZuPlayerController->ShowWeaponProperty(true);
		//设置背包剩余容量UI
		YuanZuPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
		YuanZuPlayerController->SetWeaponProperty(EquippedWeapon);
	}

	if (EquippedWeapon->PickUpSound)
	{
		//播放拾取武器音效
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->PickUpSound, YuanZuCharacter->GetActorLocation());
	}
	if (EquippedWeapon->IsEmpty())
	{
		//如果拾取的武器弹夹是空的就换弹
		if (bAiming)
		{
			bAiming = false;
			Reload();
		}

	}

	YuanZuCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;//不让角色的旋转根据移动方向走
	YuanZuCharacter->bUseControllerRotationYaw = true;//角色旋转根据摄像机旋转	

}

void UYuanZuCombatComponent::DroppedWeapon()
{
	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	
	if (YuanZuCharacter == nullptr || EquippedWeapon == nullptr || YuanZuPlayerController == nullptr) return;

	AYuanZuWeapon* OldWeapon = EquippedWeapon;
	EquippedWeapon = nullptr;

	YuanZuPlayerController->SetWeaponProperty(nullptr);
	YuanZuPlayerController->ShowWeaponProperty(false);

	if (OldWeapon)
	{
		OldWeapon->DroppedWeapon();
	}
	YuanZuCharacter->bUseControllerRotationYaw = false;
	YuanZuCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
}

void UYuanZuCombatComponent::OnRep_EquippedWeapon()
{
	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;

	if (YuanZuCharacter && EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		YuanZuCharacter->SetAmmoType(EquippedWeapon);
		const USkeletalMeshSocket* HandSocket = YuanZuCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, YuanZuCharacter->GetMesh());
		}

		if (EquippedWeapon->PickUpSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->PickUpSound, YuanZuCharacter->GetActorLocation());
		}
		if (YuanZuPlayerController)
		{
			//设置背包剩余容量UI
			YuanZuPlayerController->ShowWeaponProperty(true);
			YuanZuPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
			YuanZuPlayerController->SetWeaponProperty(EquippedWeapon);
		}

		YuanZuCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		YuanZuCharacter->bUseControllerRotationYaw = true;
	}
	else
	{
		//隐藏武器属性UI
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetWeaponProperty(nullptr);
			YuanZuPlayerController->ShowWeaponProperty(false);
		}

		YuanZuCharacter->bUseControllerRotationYaw = false;
		YuanZuCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void UYuanZuCombatComponent::OnRep_CarriedAmmo()
{
	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	if (YuanZuPlayerController)
	{
		YuanZuPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UYuanZuCombatComponent::Reload()
{
	UE_LOG(LogYuanZu, Warning, TEXT("%d"), CarriedAmmo);
	if (CarriedAmmo > 0)
	{
		switch (EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_TJBQ:
			if (EquippedWeapon->GetAmmo() != TJBQ_StartAmmo)
			{
				if (YuanZuCharacter->bIsReload) return;
				ServerReload();
			}
			break;
		case EWeaponType::EWT_HJT:
			if (EquippedWeapon->GetAmmo() != HJT_StartAmmo)
			{
				if (YuanZuCharacter->bIsReload) return;
				ServerReload();
			}
			break;
		case EWeaponType::EWT_SQ:
			if (EquippedWeapon->GetAmmo() != SQ_StartAmmo)
			{
				if (YuanZuCharacter->bIsReload) return;
				ServerReload();
			}
		case EWeaponType::EWT_JJBQ:
			if (EquippedWeapon->GetAmmo() != SQ_StartAmmo)
			{
				if (YuanZuCharacter->bIsReload) return;
				ServerReload();
			}
			break;
		}
	}

}

void UYuanZuCombatComponent::ServerReload_Implementation()
{
	if (!YuanZuCharacter || !EquippedWeapon) return;

	MulticastReload();
}

void UYuanZuCombatComponent::MulticastReload_Implementation()
{
	if (YuanZuCharacter && EquippedWeapon)
	{
		YuanZuCharacter->PlayReloadMontage();
		YuanZuCharacter->bIsReload = true;
	}
}

void UYuanZuCombatComponent::UpdateAmmoCount()
{
	if (!YuanZuCharacter || !EquippedWeapon) return;

	//装填弹药的数量
	int32 ReloadAmount = AmountToReload();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		//背包剩余容量 = 武器类型的子弹 - 装填弹药的数量
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
	}

	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	if (YuanZuPlayerController)
	{
		YuanZuPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

bool UYuanZuCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;

	return !EquippedWeapon->IsEmpty() && bCanFire && !YuanZuCharacter->bIsReload;
}

void UYuanZuCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;

		ServerFire(AimHitResult.ImpactPoint);
		if (EquippedWeapon)
		{		
			CrosshairShootingFactor += 0.5f;
		}

		StartFireTimer();
	}

}

void UYuanZuCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || YuanZuCharacter == nullptr) return;

	YuanZuCharacter->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UYuanZuCombatComponent::StopFireTimer, EquippedWeapon->FireDelay);

}			

void UYuanZuCombatComponent::StopFireTimer()
{
	if (EquippedWeapon == nullptr || YuanZuCharacter == nullptr) return;

	bCanFire = true;

	//只有在自动开火模式下，并且玩家仍然在按住开火键时，才重新开始开火计时器	
	if (bFire && EquippedWeapon->bAutoFire)
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		if (bAiming)
		{
			bAiming = false;
			Reload();
		}
	}
}

void UYuanZuCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_TJBQ, TJBQ_StartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_HJT, HJT_StartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SQ, SQ_StartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_JJBQ, JJBQ_StartAmmo);
}

void UYuanZuCombatComponent::StartFire()
{
	if (bFire && EquippedWeapon == nullptr) return;

	bFire = true;
	FHitResult NewHitResult;
	AimHitResult = NewHitResult;
	TraceUnderCrosshairs(AimHitResult);

	Fire();
}

void UYuanZuCombatComponent::StopFire()
{
	if (!bFire) return;

	bFire = false;

	StopFireTimer();

	ServerStopFire();

}

void UYuanZuCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	MulticastFire(HitTarget);
}

void UYuanZuCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(HitTarget);
	}

}

void UYuanZuCombatComponent::ServerStopFire_Implementation()
{
	bFire = false;
	MulticastStopFire();
}

void UYuanZuCombatComponent::MulticastStopFire_Implementation()
{
	if (EquippedWeapon == nullptr) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->StopFireMontage();
	}

}
