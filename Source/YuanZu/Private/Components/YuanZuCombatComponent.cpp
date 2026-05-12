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
#include "Weapons/YuanZuSnipeProjectileWeapon.h"
#include "Weapons/YuanZuAntitankProjectileWeapon.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/DecalComponent.h"
#include "Weapons/YuanZuAntitankProjectile.h"
#include "Items/YuanZuPickupItem.h"
#include "Items/YuanZuAmmoPickup.h"

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
		if (EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SLD && !bIsThrow && bIsLift)
		{
			DrawAntitankPath();
		}
	}

}

void UYuanZuCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UYuanZuCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UYuanZuCombatComponent, bAiming);
	DOREPLIFETIME(UYuanZuCombatComponent, bFire); 
	DOREPLIFETIME(UYuanZuCombatComponent, bIsLift);
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
		}
		FVector End = Start + CrosshairWorldDirection * 80000.f;

		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

		HUDPackage.CrosshairColor = FLinearColor::White;

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}
	}
}

void UYuanZuCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!YuanZuCharacter || !YuanZuCharacter->Controller) return;

	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	if (YuanZuPlayerController)
	{
		YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(YuanZuPlayerController->GetHUD()) : YuanZuHUD;
		if (YuanZuHUD)
		{
			HUDPackage.CrosshairsCenter = CrosshairCenter;
			HUDPackage.CrosshairsLeft = nullptr;
			HUDPackage.CrosshairsRight = nullptr;
			HUDPackage.CrosshairsTop = nullptr;
			HUDPackage.CrosshairsBottom = nullptr;

			if (EquippedWeapon)
			{
				if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_JJBQ && bAiming)
				{
					HUDPackage.CrosshairsCenter = nullptr;
					HUDPackage.CrosshairsLeft = nullptr;
					HUDPackage.CrosshairsRight = nullptr;
					HUDPackage.CrosshairsTop = nullptr;
					HUDPackage.CrosshairsBottom = nullptr;
				}
				else
				{
					HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
					HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
					HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairRight;
					HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairTop;
					HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
				}
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
	if (!EquippedWeapon) return 0;
	//弹夹可装填的弹药数量（弹夹最大容量 - 剩余子弹数量）
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetAmmoType()))
	{
		//背包携带的弹药量
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetAmmoType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		//如果背包剩余的数量少就都给弹夹。如果弹夹数量小，那就补弹夹可以补的那部分
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UYuanZuCombatComponent::FireAntitank()
{
	if (!EquippedWeapon)return;

	FVector LaunchVelocity = GetAntitankLaunchVelocity();

	AYuanZuAntitankProjectileWeapon* AntitankProjectileWeapon = Cast<AYuanZuAntitankProjectileWeapon>(EquippedWeapon);
	if (AntitankProjectileWeapon)
	{
		AntitankProjectileWeapon->FireAntitank(LaunchVelocity);
	}
}

void UYuanZuCombatComponent::SetAntitankState()
{
	if (!YuanZuCharacter) return;

	if (!YuanZuCharacter->HasAuthority())
	{
		ServerSetAntitankState();
		return;
	}
	//处理手雷投掷后的武器状态
	HandleAntitankState();
}

void UYuanZuCombatComponent::HandleAntitankState()
{
	if (!EquippedWeapon) return;

	if (EquippedWeapon->GetAmmo() == 0 && GetCarriedAmmoByType(EquippedWeapon->GetAmmoType()) == 0)
	{
		//当前弹药=0 携带的当前武器类型的弹药也=0
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;

		YuanZuPlayerController = YuanZuPlayerController == nullptr && YuanZuCharacter ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetWeaponProperty(nullptr);
			YuanZuPlayerController->ShowWeaponProperty(false);
		}
	}
}

void UYuanZuCombatComponent::SpawnAntitank()
{
	if (!EquippedWeapon)return;

	FVector StartLocation = GetAntitankStartLocation();
	FVector LaunchVelocity = GetAntitankLaunchVelocity();

	AYuanZuAntitankProjectileWeapon* AntitankProjectileWeapon = Cast<AYuanZuAntitankProjectileWeapon>(EquippedWeapon);
	if (AntitankProjectileWeapon)
	{
		AntitankProjectileWeapon->SpawnAntitank(StartLocation, LaunchVelocity);
	}
}

void UYuanZuCombatComponent::CencelDetonation()
{
	AYuanZuAntitankProjectileWeapon* AntitankProjectileWeapon = Cast<AYuanZuAntitankProjectileWeapon>(EquippedWeapon);
	if (AntitankProjectileWeapon)
	{
		AntitankProjectileWeapon->CencelDetonation();
	}
}

void UYuanZuCombatComponent::SetThrowMode(bool& bHeight)
{
	bIsHighThrow = bHeight;
}

void UYuanZuCombatComponent::PickupAmmo(EAmmoType InAmmoType, int32 InAmmoCount)
{
	if (!YuanZuCharacter || !YuanZuCharacter->GetOverlappingItem())return;

	AYuanZuAmmoPickup* YZAmmo = Cast<AYuanZuAmmoPickup>(YuanZuCharacter->GetOverlappingItem());
	if (YZAmmo)
	{
		AddCarriedAmmo(InAmmoType, InAmmoCount);
		YZAmmo->Destroy();
		YuanZuCharacter->SetOVerlappingItem(nullptr);
	}
	else
	{
		UE_LOG(LogYuanZu, Warning, TEXT("PickupAmmo Cast Failed. OverlappingItem=%s, Class=%s"),
			*GetNameSafe(YuanZuCharacter->GetOverlappingItem()),
			*GetNameSafe(YuanZuCharacter->GetOverlappingItem()->GetClass()));
	}

}

void UYuanZuCombatComponent::AddCarriedAmmo(EAmmoType InAmmoType, int32 InAmmoCount)
{
	int32& Ammo = CarriedAmmoMap.FindOrAdd(InAmmoType);
	Ammo += InAmmoCount;

	if (EquippedWeapon && EquippedWeapon->GetAmmoType() == InAmmoType)
	{
		CarriedAmmo = Ammo;

		if (YuanZuCharacter)
		{
			YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
			if (YuanZuPlayerController)
			{
				YuanZuPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
			}
		}
	}
}

void UYuanZuCombatComponent::SetWeaponSlot(AYuanZuWeapon* InWeapon)
{
	switch (InWeapon->GetWeaponSlot())
	{
	case EWeaponSlot::EWS_Primary:
		break;
	case EWeaponSlot::EWS_Pistol:
		break;
	case EWeaponSlot::EWS_Melee:
		break;
	case EWeaponSlot::EWS_Throwable:
		break;
	case EWeaponSlot::EWS_Max:
		break;
	default:
		break;
	}
}

FVector UYuanZuCombatComponent::GetAntitankStartLocation() const
{
	return YuanZuCharacter->GetMesh()->GetSocketLocation(FName("RightHandSocket"));
}

FVector UYuanZuCombatComponent::GetAntitankLaunchVelocity() const
{
	const float ThrowSpeed = bIsHighThrow ? 1500.f : 900.f;
	const float UpStrength = bIsHighThrow ? 0.15f : 0.15f;

	FVector ThrowDir = YuanZuCharacter->GetControlRotation().Vector();
	ThrowDir = (ThrowDir + FVector::UpVector * UpStrength).GetSafeNormal();

	return ThrowDir * ThrowSpeed;
}

void UYuanZuCombatComponent::SetThrow(bool bThrow)
{
	if (EquippedWeapon->GetAmmo() < 1)return;

	if (bThrow)
	{
		if (bIsLift)
		{
			//停止举起动画，播放投掷动画
			bIsThrow = true;
			bCanThrow = false;
			StopLift();
		}
	}
	else
	{
		//放下手雷
		bIsThrow = false;
		CencelLift();
	}
}

void UYuanZuCombatComponent::StartLift()
{
	if (bIsLift || !YuanZuCharacter)return;

	CencelLift();
}

void UYuanZuCombatComponent::StopLift()
{
	if (!YuanZuCharacter)return;

	if (bIsThrow)
	{
		//播放投掷动画
		bIsLift = false;
		bIsThrow = false;
		ClearProjectileLine();
		YuanZuCharacter->PlayAntitankFireMontage(bIsHighThrow);
		ServerAntitank(bIsHighThrow);
	}
}

void UYuanZuCombatComponent::CencelLift()
{
	if (!EquippedWeapon)return;
	AYuanZuAntitankProjectileWeapon* APWeapon = Cast<AYuanZuAntitankProjectileWeapon>(EquippedWeapon);
	if (APWeapon)
	{
		if (YuanZuCharacter)
		{
			if (bIsLift)
			{
				CencelDetonation();
				ClearProjectileLine();
			}
			else
			{
				YuanZuCharacter->PlayPullBoltMontage();
			}
			ServerPullBolt();
		}
	}

}

void UYuanZuCombatComponent::ServerPullBolt_Implementation()
{
	if (bIsLift)
	{
		bIsLift = false;
		ClearProjectileLine();
		return;
	}
	EquippedWeapon->GetWeaponMesh()->SetVisibility(false);

	SpawnAntitank();
	MulticastPullBolt();
}

void UYuanZuCombatComponent::MulticastPullBolt_Implementation()
{
	if (IsLocalPlayer())return;

	if (YuanZuCharacter)
	{
		//打开投掷物拉栓
		YuanZuCharacter->PlayPullBoltMontage();
	}
}

void UYuanZuCombatComponent::ServerAntitank_Implementation(bool bHighThrow)
{
	bIsLift = false;
	bIsThrow = false;
	bIsHighThrow = bHighThrow;
	MulticastAntitank(bHighThrow);
}

void UYuanZuCombatComponent::MulticastAntitank_Implementation(bool bHighThrow)
{
	if (IsLocalPlayer())return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->PlayAntitankFireMontage(bHighThrow);
	}
}

void UYuanZuCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (YuanZuCharacter && !AimSound.IsEmpty() && UnAimSound)
	{
		if (bIsAiming)
		{
			switch (EquippedWeapon->GetWeaponType())
			{
			case EWeaponType::EWT_JJBQ:
				UGameplayStatics::PlaySoundAtLocation(this, AimSound[1], YuanZuCharacter->GetActorLocation());
				break;
			default:
				UGameplayStatics::PlaySoundAtLocation(this, AimSound[0], YuanZuCharacter->GetActorLocation());
				break;
			}
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(this, UnAimSound, YuanZuCharacter->GetActorLocation());
		}

		YuanZuCharacter->SetJJBQAim(EquippedWeapon, bAiming);
		ServerSetAiming(bAiming);
	}
}

void UYuanZuCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	MulticastSetAiming(bIsAiming);
}

void UYuanZuCombatComponent::MulticastSetAiming_Implementation(bool bIsAiming)
{
	if (IsLocalPlayer())return;

	if (bIsAiming)
	{
		switch (EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_JJBQ:
			UGameplayStatics::PlaySoundAtLocation(this, AimSound[1], YuanZuCharacter->GetActorLocation());
			break;
		default:
			UGameplayStatics::PlaySoundAtLocation(this, AimSound[0], YuanZuCharacter->GetActorLocation());
			break;
		}
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(this, UnAimSound, YuanZuCharacter->GetActorLocation());
	}
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

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetAmmoType()))
	{
		//将武器类型的子弹数量放到背包剩余容量
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetAmmoType()];
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
}

void UYuanZuCombatComponent::DroppedWeapon()
{
	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	
	if (!YuanZuCharacter || !EquippedWeapon || !YuanZuPlayerController) return;

	AYuanZuWeapon* OldWeapon = EquippedWeapon;
	EquippedWeapon = nullptr;
	YuanZuPlayerController->SetWeaponProperty(nullptr);
	YuanZuPlayerController->ShowWeaponProperty(false);

	if (OldWeapon)
	{
		OldWeapon->DroppedWeapon();
		if (OldWeapon->GetWeaponType() == EWeaponType::EWT_SLD)
		{
			ClearProjectileLine();
		}
	}
}

void UYuanZuCombatComponent::ServerFireAntitank_Implementation()
{
	FireAntitank();
}

void UYuanZuCombatComponent::ServerSetAntitankState_Implementation()
{
	HandleAntitankState();
}

void UYuanZuCombatComponent::OnRep_EquippedWeapon()
{
	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;

	if (EquippedWeapon)
	{
		if (YuanZuCharacter)
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
		}
	}
	else
	{
		//隐藏武器属性UI
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetWeaponProperty(nullptr);
			YuanZuPlayerController->ShowWeaponProperty(false);
		}
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
	if (!YuanZuCharacter || !EquippedWeapon) return;

	if (CarriedAmmo > 0 && EquippedWeapon->GetAmmo() < EquippedWeapon->GetMagCapacity())
	{
		bAiming = false;

		AYuanZuSnipeProjectileWeapon* SnipeProjectileWeapon = Cast<AYuanZuSnipeProjectileWeapon>(EquippedWeapon);
		if (SnipeProjectileWeapon && SnipeProjectileWeapon->Scope)
		{
			SnipeProjectileWeapon->Scope->RemoveFromParent();
		}

		YuanZuCharacter->PlayReloadMontage(bAiming);
		YuanZuCharacter->bIsReload = true;

		UE_LOG(LogYuanZu, Warning, TEXT("携带的弹药数量[%d]"), CarriedAmmo);

		//携带的弹药 > 0 且 当前装备的武器的弹药数量 < 弹夹㓯容量
		//例如30 && 15 < 30
		if (!YuanZuCharacter->bIsReload) return;
		ServerReload();
	}
}

void UYuanZuCombatComponent::ServerReload_Implementation()
{
	if (!YuanZuCharacter || !EquippedWeapon) return;

	MulticastReload();
}

void UYuanZuCombatComponent::MulticastReload_Implementation()
{
	if (IsLocalPlayer())return;

	if (YuanZuCharacter && EquippedWeapon)
	{
		YuanZuCharacter->PlayReloadMontage(bAiming);
		YuanZuCharacter->bIsReload = true;
	}
}

void UYuanZuCombatComponent::UpdateAmmoCount()
{
	if (!YuanZuCharacter || !EquippedWeapon) return;

	//装填弹药的数量
	int32 ReloadAmount = AmountToReload();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetAmmoType()))
	{
		//背包剩余容量 = 武器类型的子弹 - 装填弹药的数量
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetAmmoType()] -= ReloadAmount;
	}

	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
	if (YuanZuPlayerController)
	{
		YuanZuPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

bool UYuanZuCombatComponent::ConsumeCarriedAmmo(EAmmoType AmmoType, int32 AmmoAmount)
{
	if (AmmoAmount <= 0) return false;

	//根据武器类型获取携带的弹药数量
	int32* CurrentCarriedAmmo = CarriedAmmoMap.Find(AmmoType);
	//如果当前携带的弹药 < 对应的武器类型的弹药数量就返回false
	if (!CurrentCarriedAmmo || *CurrentCarriedAmmo < AmmoAmount) return false;

	*CurrentCarriedAmmo -= AmmoAmount;//当前弹药-1

	if (EquippedWeapon && EquippedWeapon->GetAmmoType() == AmmoType)
	{
		CarriedAmmo = *CurrentCarriedAmmo;

		if (YuanZuCharacter)
		{
			YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
			if (YuanZuPlayerController)
			{
				YuanZuPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
			}
		}
	}

	return true;
}

int32 UYuanZuCombatComponent::GetCarriedAmmoByType(EAmmoType AmmoType) const
{
	if (const int32* CurrentCarriedAmmo = CarriedAmmoMap.Find(AmmoType))
	{
		return *CurrentCarriedAmmo;
	}

	return 0;
}

void UYuanZuCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EAmmoType::EAT_762, ZDBQ_StartAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_556, TJBQ_StartAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_2KJ, HJT_StartAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_9mm, SQ_StartAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_425, JJBQ_StartAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_40MM, LDQ_StartAmmo);
	CarriedAmmoMap.Emplace(EAmmoType::EAT_G67, SLD_StartAmmo);
}

void UYuanZuCombatComponent::StartAttack()
{
	if (YuanZuCharacter)
	{
		YuanZuCharacter->PlayAttackMontage();
	}
	ServerAttack();
}

//绘制投掷物路径
void UYuanZuCombatComponent::DrawAntitankPath()
{
	if (YuanZuCharacter)
	{
		//预测弹道路径参数
		FPredictProjectilePathParams Params;

		//起点位置：手的位置
		Params.StartLocation = GetAntitankStartLocation();

		//初速度：角色朝向 + 上方偏移 * 投掷力度
		Params.LaunchVelocity = GetAntitankLaunchVelocity();//发射速度
		Params.bTraceWithCollision = true;//碰到物体就停止
		Params.MaxSimTime = 3.f;//模拟时间
		Params.SimFrequency = 30.f;//采样频率，越高越平滑
		Params.DrawDebugType = EDrawDebugTrace::None;//不用debug线，我们自己画
		Params.TraceChannel = ECC_WorldStatic;
		Params.bTraceWithChannel = true;//通过通道追踪

		//忽略自身
		Params.ActorsToIgnore.Add(YuanZuCharacter);

		//预测弹道路径结果
		FPredictProjectilePathResult Result;
		//预测弹道路径
		UGameplayStatics::PredictProjectilePath(this, Params, Result);

		// Result.PathData 包含所有轨迹点
		// Result.HitResult 是落点

		UpdateProjectileLine(Result.PathData, Result.HitResult);
	}
}

void UYuanZuCombatComponent::UpdateProjectileLine(const TArray<FPredictProjectilePathPointData>& PathData, const FHitResult& HitResult)
{
	if (!ParabolicPointMaterial)return;
	//清空旧的样条线上的所有路径
	YuanZuCharacter->GetProjectileLine()->ClearSplinePoints();
	for (USplineMeshComponent* NewMesh : LineMeshes)
	{
		//销毁样条线上的所有网格
		NewMesh->DestroyComponent();
	}
	LineMeshes.Empty();//清空样条线上的所有网格数组

	//添加轨迹点到样条
	for (int32 i = 0; i < PathData.Num(); i++)
	{
		//添加样条点（位置，样条坐标控件）位置：先获取样条线上的点；获取该点在世界控件下的位置
		YuanZuCharacter->GetProjectileLine()->AddSplinePoint(PathData[i].Location, ESplineCoordinateSpace::World);
	}

	//为每两个点之间创建一个网格
	//GetNumberOfSplinePoints获取样条点的数量
	for (int32 i = 0; i < YuanZuCharacter->GetProjectileLine()->GetNumberOfSplinePoints() - 1; i++)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		SplineMesh->SetStaticMesh(ParabolicLineMesh);
		SplineMesh->SetMobility(EComponentMobility::Movable);//将样条线网格的移动性设置为可移动的，不然玩家移动的时候样条网格不动
		SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//设置无碰撞
		SplineMesh->SetStartScale(FVector2D(3.f, 3.f));
		SplineMesh->SetEndScale(FVector2D(3.f, 3.f));
		if (LineMeshesMaterial)
		{
			SplineMesh->SetMaterial(0, LineMeshesMaterial);
		}
		UE_LOG(LogTemp, Warning, TEXT("LineMeshesMaterial=%s, Mesh=%s, SlotNum=%d, CurrentMat=%s"),
			*GetNameSafe(LineMeshesMaterial),
			*GetNameSafe(ParabolicLineMesh),
			SplineMesh->GetNumMaterials(),
			*GetNameSafe(SplineMesh->GetMaterial(0)));
		FVector StartPosition;//开始位置
		FVector StartTangent;//开始位置的切线
		FVector EndPosition;//结束位置
		FVector EndTangent;//结束位置的切线
		//获取每个点的位置以及切线方向
		//开始位置：第0个点开始位置和切线
		YuanZuCharacter->GetProjectileLine()->GetLocationAndTangentAtSplinePoint(i, StartPosition, StartTangent, ESplineCoordinateSpace::World);
		//结束位置：第0+1个点开始位置和切线
		YuanZuCharacter->GetProjectileLine()->GetLocationAndTangentAtSplinePoint(i + 1, EndPosition, EndTangent, ESplineCoordinateSpace::World);
		//设置开始的位置和切线 结束位置的位置和切线
		SplineMesh->SetStartAndEnd(StartPosition, StartTangent, EndPosition, EndTangent);

		//动态创建的组件必须手动注册才能渲染 将样条线网格添加到世界中来
		SplineMesh->RegisterComponentWithWorld(GetWorld());

		//将设置好的的样条网格添加给在世界中绘制的网格
		LineMeshes.Add(SplineMesh);
	}
	if (!ParabolicPointDecal)
	{
		ParabolicPointDecal = UGameplayStatics::SpawnDecalAtLocation(this,
			ParabolicPointMaterial,
			FVector(10, 10, 10),
			HitResult.Location,
			FRotator(-90, 0, 0), 0.f);
	}
	else
	{
		ParabolicPointDecal->SetWorldLocation(HitResult.Location);
		ParabolicPointDecal->SetWorldRotation(FRotator(-90, 0, 0), 0.f);
		ParabolicPointDecal->SetVisibility(true);
	}
}

void UYuanZuCombatComponent::ClearProjectileLine()
{
	if (YuanZuCharacter->GetProjectileLine())
	{
		YuanZuCharacter->GetProjectileLine()->ClearSplinePoints();
	}

	for (USplineMeshComponent* NewMesh : LineMeshes)
	{
		if (NewMesh)
		{
			NewMesh->DestroyComponent();
		}
	}
	LineMeshes.Empty();

	if (ParabolicPointDecal)
	{
		ParabolicPointDecal->DestroyComponent();
		ParabolicPointDecal = nullptr;
	}
}

bool UYuanZuCombatComponent::IsLocalPlayer()
{
	APawn* Pawn = Cast<APawn>(GetOwner());

	if (Pawn)
	{
		return Pawn->IsLocallyControlled();
	}
	return false;
}

void UYuanZuCombatComponent::ServerAttack_Implementation()
{
	MulticastAttack();
}

void UYuanZuCombatComponent::MulticastAttack_Implementation()
{
	if (IsLocalPlayer())return;
	if (YuanZuCharacter)
	{
		YuanZuCharacter->PlayAttackMontage();
	}
}

bool UYuanZuCombatComponent::CanFire()
{
	if (!EquippedWeapon) return false;

	return !EquippedWeapon->IsEmpty() && bCanFire && !YuanZuCharacter->bIsReload;
}

void UYuanZuCombatComponent::Fire()
{
	if (CanFire() && YuanZuCharacter)
	{
		bCanFire = false;

		YuanZuCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(AimHitResult.ImpactPoint);
		//通知服务器执行多播
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
	if (!EquippedWeapon || !YuanZuCharacter) return;

	YuanZuCharacter->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UYuanZuCombatComponent::StopFireTimer, EquippedWeapon->FireDelay);

}			

void UYuanZuCombatComponent::StopFireTimer()
{
	if (!EquippedWeapon || !YuanZuCharacter) return;

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

void UYuanZuCombatComponent::StartFire()
{
	if (bFire && !EquippedWeapon) return;

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
	if (YuanZuCharacter)
	{
		YuanZuCharacter->StopFireMontage();
	}
	ServerStopFire();
}

void UYuanZuCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	MulticastFire(HitTarget);
}

void UYuanZuCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	if (!EquippedWeapon || IsLocalPlayer()) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(HitTarget);
	}

}

void UYuanZuCombatComponent::ServerStopFire_Implementation()
{
	if (EquippedWeapon->GetWeaponType() != EWeaponType::EWT_SLD)
	{
		bFire = false;
	}
	MulticastStopFire();
}

void UYuanZuCombatComponent::MulticastStopFire_Implementation()
{
	if (!EquippedWeapon	|| IsLocalPlayer()) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->StopFireMontage();
	}

}
