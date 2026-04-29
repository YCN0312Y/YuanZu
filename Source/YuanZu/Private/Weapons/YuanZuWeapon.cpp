// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/YuanZuCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/YuanZuBulletShell.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Gameplay/YuanZuPlayerController.h"
#include "UI/Weapon/YuanZuPickUp.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

// Sets default values
AYuanZuWeapon::AYuanZuWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;//允许复制
	SetReplicateMovement(true);//复制移动
	//武器骨骼网格体----------
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	//碰撞
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);//将所有碰撞通道的碰撞相应设置为阻挡
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);//仅将Pawn通道的碰撞相应设置为武略
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//球体区域----------
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);//将所有碰撞通道的碰撞相应设置为阻挡
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);//仅将Pawn通道的碰撞相应设置为武略
	AreaSphere->SetSphereRadius(64.f);

	//拾取组件
	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidget->SetupAttachment(RootComponent);
	PickUpWidget->SetWidgetSpace(EWidgetSpace::Screen);
	PickUpWidget->SetDrawAtDesiredSize(true);

	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	AmmoMesh->SetupAttachment(RootComponent);
	AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AYuanZuWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (AmmoMesh)
	{
		AmmoMesh->SetVisibility(true);
	}

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
		PickUpWidget->SetWidgetClass(PickUp);
		UYuanZuPickUp* NewPickUp = Cast<UYuanZuPickUp>(PickUpWidget->GetUserWidgetObject());
		if (NewPickUp)
		{
			NewPickUp->SetWeapon(this);
		}
	}
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AYuanZuWeapon::OnSphereBeginOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AYuanZuWeapon::OnSphereEndOverlap);
	}
	Ammo = MagCapacity;
	
}

// Called every frame
void AYuanZuWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYuanZuWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYuanZuWeapon, WeaponState);
	DOREPLIFETIME(AYuanZuWeapon, Ammo);
}

void AYuanZuWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AYuanZuCharacterBase* YuanZuCharacterBase = Cast<AYuanZuCharacterBase>(OtherActor);
	if (YuanZuCharacterBase)
	{
		YuanZuCharacterBase->SetOverlappingWeapon(this);
	}
	
}

void AYuanZuWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AYuanZuCharacterBase* YuanZuCharacterBase = Cast<AYuanZuCharacterBase>(OtherActor);
	if (YuanZuCharacterBase)
	{
		YuanZuCharacterBase->SetOverlappingWeapon(nullptr);
	}
}

bool AYuanZuWeapon::IsEmpty()
{
	return Ammo <= 0;
}

void AYuanZuWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	UE_LOG(LogYuanZu, Warning, TEXT("Ammo[%d]"), Ammo);
	SetHUDAmmo();
}

void AYuanZuWeapon::SetWeaponMaterial(bool bIsVisible)
{
	if (bIsVisible)
	{
		WeaponMesh->SetMaterial(0, DefaultMaterial);
	}
	else
	{
		WeaponMesh->SetMaterial(0, ReloadMaterial);
	}
}

void AYuanZuWeapon::ShowAmmoMesh(bool bIsVisible)
{
	switch (WeaponType)
	{
	case EWeaponType::EWT_TJBQ:
		SetWeaponMaterial(bIsVisible);
		break;
	case EWeaponType::EWT_LDQ:
		break;
	case EWeaponType::EWT_BS:
		break;
	case EWeaponType::EWT_SQ:
				SetWeaponMaterial(bIsVisible);
		break;
	case EWeaponType::EWT_HJT:
		AmmoMesh->SetVisibility(bIsVisible);
		break;
	case EWeaponType::EWT_SDQ:
		break;
	case EWeaponType::EWT_JJBQ:
		SetWeaponMaterial(bIsVisible);
		break;
	case EWeaponType::EWT_MAX:
		break;
	default:
		break;
	}
}

void AYuanZuWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AYuanZuWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (!Owner)
	{
		YuanZuOwnerCharacter = nullptr;
		YuanZuOwnerPlayerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}

void AYuanZuWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}
}

void AYuanZuWeapon::DroppedWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);//丢掉武器
	SetOwner(nullptr);
	YuanZuOwnerCharacter = nullptr;
	YuanZuOwnerPlayerController = nullptr;
}

void AYuanZuWeapon::SetHUDAmmo()
{
	YuanZuOwnerCharacter = YuanZuOwnerCharacter == nullptr ? Cast<AYuanZuCharacterBase>(GetOwner()) : YuanZuOwnerCharacter;

	if (YuanZuOwnerCharacter)
	{
		YuanZuOwnerPlayerController = YuanZuOwnerPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuOwnerCharacter->Controller) : YuanZuOwnerPlayerController;
		if (YuanZuOwnerPlayerController)
		{
			YuanZuOwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AYuanZuWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AYuanZuWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (BulletShellClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(TEXT("AmmoEject"));

		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);//弹壳的变换
			GetWorld()->SpawnActor<AYuanZuBulletShell>(BulletShellClass, SocketTransform.GetLocation(), SocketTransform.Rotator());//生成弹壳

		}

	}
	SpendRound();
}

void AYuanZuWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);//启用物理模拟
		WeaponMesh->SetEnableGravity(false);//启用重力
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);//启用物理模拟
		WeaponMesh->SetEnableGravity(true);//启用重力
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AYuanZuWeapon::OnRep_WeaponState()
{

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);//启用物理模拟
		WeaponMesh->SetEnableGravity(false);//启用重力
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);//启用物理模拟
		WeaponMesh->SetEnableGravity(true);//启用重力
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}
