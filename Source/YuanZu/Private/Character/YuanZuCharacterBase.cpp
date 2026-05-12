#include "Character/YuanZuCharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "Weapons/YuanZuWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Components/YuanZuCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PhysicsVolume.h"
#include "Components/BoxComponent.h"
#include "Animation/YuanZuAnimInstance.h"
#include "YuanZu/YuanZu.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "Gameplay/YuanZuPlayerController.h"
#include "Gameplay/YuanZuHUD.h"
#include "Components/TextBlock.h"
#include "Gameplay/YuanZuGameMode.h"
#include "TimerManager.h"
#include "Gameplay/YuanZuPlayerState.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/YuanZuSnipeProjectileWeapon.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "Components/SplineComponent.h"
#include "Items/YuanZuPickupItem.h"

AYuanZuCharacterBase::AYuanZuCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	//弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 40.f);

	//摄像机
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	AmmoMesh->SetupAttachment(GetMesh());
	AmmoMesh->SetVisibility(false);
	AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bUseControllerRotationYaw = true;
	//生成碰撞处理方法为：尝试调整位置 但始终会重生
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//战斗组件
	Combat = CreateDefaultSubobject<UYuanZuCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	ProjectileLine = CreateDefaultSubobject<USplineComponent>(TEXT("ProjectileLine"));
	ProjectileLine->SetupAttachment(GetMesh());

	//时间轴
	WalkingTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WalkingTimeline"));
	SwimTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SwimTimeline"));

	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->bOrientRotationToMovement = false;//方向转至移动角度
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;//允许蹲下
	GetCharacterMovement()->SetCrouchedHalfHeight(60.0f);//蹲下目标大小
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;//蹲下最大移动速度
	GetCharacterMovement()->SetIsReplicated(true);//复制
	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);
	GetCharacterMovement()->MaxSwimSpeed = 0.f;
	GetCharacterMovement()->SetWalkableFloorAngle(60.f);

	NetUpdateFrequency = 66.f;//网络更新频率
	MinNetUpdateFrequency = 33.f;//网络最小更新频率

	//C++基类
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	TurningInPlace = ETurningInPlace::ETIP_NotTurnin;

}

void AYuanZuCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (AmmoMesh)
	{
		AmmoMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("ReloadSocket"));
	}

	if (CameraBoom)
	{
		DefaultTargetArmLength = CameraBoom->TargetArmLength;//设置默认弹簧臂长度
		DefaultCameraBoomSocketOffset = CameraBoom->SocketOffset;//设置默认摄像机偏移
		DefaultCameraBoomRelativeLocation = CameraBoom->GetRelativeLocation();//设置默认弹簧臂位置
		TargetCameraLocation = DefaultCameraBoomRelativeLocation.Z;
		TargetCrouchArmLength = DefaultTargetArmLength;
	}

	if (WalkRetardance && WalkingTimeline)
	{
		OnRunWalkTimelineUpdateDelegate.BindUFunction(this, FName("OnRunWalkTimelineUpdate"));
		WalkingTimeline->AddInterpFloat(WalkRetardance, OnRunWalkTimelineUpdateDelegate);
	}
	if (SwimRetardance && SwimTimeline)
	{
		OnSwimTimelineUpdateDelegate.BindUFunction(this, FName("OnSwimTimelineUpdate"));
		SwimTimeline->AddInterpFloat(SwimRetardance, OnSwimTimelineUpdateDelegate);

	}
	UpdateHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AYuanZuCharacterBase::SetDemage);
	}
	AYuanZuPlayerState* YuanZuPS = GetPlayerState<AYuanZuPlayerState>();
	if (YuanZuPS)
	{
		SetCharacterMaterial(YuanZuPS->GetTeamType());
	}
	AddAttackAnimName();
}

void AYuanZuCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);

	SetCameraLocation(DeltaTime);
}

void AYuanZuCharacterBase::Destroyed()
{
	Super::Destroyed();

	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void AYuanZuCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AYuanZuPlayerState* YuanZuPS = GetPlayerState<AYuanZuPlayerState>();
	if (YuanZuPS)
	{
		SetCharacterMaterial(YuanZuPS->GetTeamType());
	}
}

//获取终身重复使用的道具
void AYuanZuCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DO = 执行/定义
	//REPL = Replication(网络复制)
	//LIFETIME = 生命周期
	//在对象的整个生命周期内进行网络复制
	//COND_OwnerOnly：此属性仅能发送给该角色的所有者
	//仅让触发重叠的玩家显示Widget
	DOREPLIFETIME_CONDITION(AYuanZuCharacterBase, OverlappingWeapon, COND_OwnerOnly);//网络复制
	DOREPLIFETIME_CONDITION(AYuanZuCharacterBase, OverlappingItem, COND_OwnerOnly);//网络复制
	DOREPLIFETIME(AYuanZuCharacterBase, bIsWalking);
	DOREPLIFETIME(AYuanZuCharacterBase, bIsSwimming);
	DOREPLIFETIME(AYuanZuCharacterBase, TurningInPlace);
	DOREPLIFETIME(AYuanZuCharacterBase, CurrentHealth);
	DOREPLIFETIME(AYuanZuCharacterBase, bIsDeath);
	
}

void AYuanZuCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->YuanZuCharacter = this;
	}
}

void AYuanZuCharacterBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (!CameraBoom)return;

	TargetCrouchArmLength = CrouchArmLength;
	TargetCameraLocation = CrouchCameraLocation;

	if (CrouchSound && GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CrouchSound, GetActorLocation());
	}
}

void AYuanZuCharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (!CameraBoom)return;
	TargetCrouchArmLength = DefaultTargetArmLength;
	TargetCameraLocation = DefaultCameraBoomRelativeLocation.Z;

	if (UnCrouchSound && GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::PlaySoundAtLocation(this, UnCrouchSound, GetActorLocation());
	}
}

void AYuanZuCharacterBase::Jump()
{
	Super::Jump();
}

void AYuanZuCharacterBase::StopJumping()
{
	Super::StopJumping();
}

void AYuanZuCharacterBase::SetCameraLocation(float DeltaTime)
{
	if (!CameraBoom) return;
	float ArmLength = CameraBoom->TargetArmLength;
	FVector Offset = CameraBoom->GetRelativeLocation();
	Offset.Z = FMath::FInterpTo(Offset.Z, TargetCameraLocation, DeltaTime, CrouchInterpSpeed);
	ArmLength = FMath::FInterpTo(ArmLength, TargetCrouchArmLength, DeltaTime, CrouchInterpSpeed);
	CameraBoom->TargetArmLength = ArmLength;
	CameraBoom->SetRelativeLocation(Offset);
}

void AYuanZuCharacterBase::AddAttackAnimName()
{
	AttackName.Add(FName(TEXT("Attack_L")));
	AttackName.Add(FName(TEXT("Attack_R")));
	AttackName.Add(FName(TEXT("Whack_L")));
	AttackName.Add(FName(TEXT("Whack_R")));
}

void AYuanZuCharacterBase::OnRep_OverlappingWidget(AYuanZuWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}


void AYuanZuCharacterBase::OnRunWalkTimelineUpdate(float Alpha)
{
	if (WalkRetardance && GetCharacterMovement() && !bIsCrouched)
	{
		if (Combat && Combat->EquippedWeapon)
		{
			int32 DefaultSpeed;
			int32 EquipHJTSpeed;

			switch (GetWeaponType())
			{
			default:
				DefaultSpeed = FMath::Lerp(600.f, 300.f, Alpha);
				GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
				UE_LOG(LogYuanZu, Warning, TEXT("EquipTJBQSpeed[%d]"), DefaultSpeed);
				break;
			case EWeaponType::EWT_HJT:
				EquipHJTSpeed = FMath::Lerp(450.f, 300.f, Alpha);
				GetCharacterMovement()->MaxWalkSpeed = EquipHJTSpeed;
				UE_LOG(LogYuanZu, Warning, TEXT("EquipHJTSpeed[%d]"), EquipHJTSpeed);
				break;
			}
		}
		else if(Combat)
		{
			//计算目标速度
			float TargetSpeed = FMath::Lerp(600.f, 300.f, Alpha);
			GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
		}
	}
}

void AYuanZuCharacterBase::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	if (IsLocallyControlled())
	{
		ViewYaw = GetControlRotation().Yaw;
	}

	FVector Velocity = GetVelocity();//获取速度
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();//获取向X和Y的速度
	bool bIsInAir = GetCharacterMovement()->IsFalling();//角色是否在空中
	FRotator AimRotation = GetAimRotation();
	float AimYaw = AimRotation.Yaw;
	float DeltaYaw = FMath::FindDeltaAngleDegrees(LastYaw, AimYaw);
	ContinuousYaw += DeltaYaw;
	ContinuousYaw = FMath::UnwindDegrees(ContinuousYaw);//将角度限制在(-180~180)之间
	LastYaw = AimYaw;

	if (Speed == 0.f && !bIsInAir)//没跑也没跳时
	{
		AO_Yaw = ContinuousYaw;

		if (AO_Yaw > 90.0f && !IsLocallyControlled())
		{
			//映射俯仰从(270~360)转为(-90~0)
			FVector2D InRange(270.f, 360.f);
			FVector2D OutRange(90.f, 0.f);
			AO_Yaw = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Yaw);//将输入范围转化为输出范围赋给AO_Pitch
		}
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurnin)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)//跑或跳时
	{
		ContinuousYaw = 0.f;
		LastYaw = AimYaw;

		StartAimRotation = FRotator(0.f, AimYaw, 0.f);
		AO_Yaw = 0.f;
		TurningInPlace = ETurningInPlace::ETIP_NotTurnin;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//映射俯仰从(270~360)转为(-90~0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);//将输入范围转化为输入范围赋给AO_Pitch
	}

}

void AYuanZuCharacterBase::ServerPlaySwimTimeline_Implementation()
{
	bIsSwimming = true;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
	}

	if (SwimTimeline)
	{
		SwimTimeline->Play();
	}
}

void AYuanZuCharacterBase::ServerReverseSwimTimeline_Implementation()
{
	bIsSwimming = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	if (SwimTimeline)
	{
		SwimTimeline->Reverse();
	}
}

void AYuanZuCharacterBase::ServerTurningInPlace_Implementation(ETurningInPlace NewState)
{
	TurningInPlace = NewState;
}

FRotator AYuanZuCharacterBase::GetAimRotation() const
{
	if (IsLocallyControlled())//是本地控制的
	{
		return GetControlRotation();
	}
	return FRotator(0.f, ViewYaw, 0.f);
}

void AYuanZuCharacterBase::TurnInPlace(float DeltaTime)
{
	if (!IsLocallyControlled())
	{
		return;
	};

	if (TurningInPlace == ETurningInPlace::ETIP_NotTurnin)
	{
		StartAimRotation = GetBaseAimRotation();

		if (AO_Yaw > 90.f)
		{
			if (HasAuthority())
			{
				TurningInPlace = ETurningInPlace::ETIP_Right;
			}
			else
			{
				ServerTurningInPlace(ETurningInPlace::ETIP_Right);
			}
		}
		else if (AO_Yaw < -90.f)
		{
			if (HasAuthority())
			{
				TurningInPlace = ETurningInPlace::ETIP_Left;
			}
			else
			{
				ServerTurningInPlace(ETurningInPlace::ETIP_Left);
			}
		}
	}


	else
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			if (HasAuthority())
			{
				TurningInPlace = ETurningInPlace::ETIP_NotTurnin;
			}
			else
			{
				ServerTurningInPlace(ETurningInPlace::ETIP_NotTurnin);
			}
			ContinuousYaw = 0.f;

			LastYaw = GetBaseAimRotation().Yaw;
		}
	}

}

void AYuanZuCharacterBase::ServerPlayWalkTimeline_Implementation()
{
	if (WalkingTimeline)
	{
		WalkingTimeline->Play();

		UE_LOG(LogYuanZu, Warning, TEXT("Walking"));
	}
}

void AYuanZuCharacterBase::ServerReverseWalkTimeline_Implementation()
{
	if (WalkingTimeline)
	{
		WalkingTimeline->Reverse();
	}
}

void AYuanZuCharacterBase::OnRep_IsWalking()
{
	if (WalkingTimeline)
	{
		if (bIsWalking)
		{
			WalkingTimeline->Play();
		}
		else
		{
			WalkingTimeline->Reverse();
		}
	}
}

void AYuanZuCharacterBase::OnRep_IsSwimming()
{
	if (!GetCharacterMovement()) return;

	if (bIsSwimming)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Swimming);

		if (SwimTimeline)
		{
			SwimTimeline->Play();
		}
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		if (SwimTimeline)
		{
			SwimTimeline->Reverse();
		}
	}

}

void AYuanZuCharacterBase::SetDemage(AActor* DemageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	//UE_LOG(LogYuanZu, Warning, TEXT("CurrentHealth[%f]"), CurrentHealth);
	UpdateHealth();
	if (CurrentHealth == 0.f)
	{
		AYuanZuGameMode* YuanZuGameMode = GetWorld()->GetAuthGameMode<AYuanZuGameMode>();
		if (YuanZuGameMode)
		{
			YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(Controller) : YuanZuPlayerController;
			AYuanZuPlayerController* AttackerPlayerController = Cast<AYuanZuPlayerController>(InstigatorController);
			YuanZuGameMode->PlayerDeath(this, YuanZuPlayerController, AttackerPlayerController);
		}
	}

}

void AYuanZuCharacterBase::Death()
{
	MulticastDeath();

	GetWorldTimerManager().SetTimer(RebirthTimerHandle, this, &AYuanZuCharacterBase::Rebirth, RebirthDelay);

}

void AYuanZuCharacterBase::MulticastDeath_Implementation()
{
	bIsDeath = true;

	GetCharacterMovement()->DisableMovement();//禁用移动
	GetCharacterMovement()->StopMovementImmediately();//立即停止移动

	if (YuanZuPlayerController)
	{
		YuanZuPlayerController->bDisableGameplay = true;
		YuanZuPlayerController->ClientShowWeaponProperty(false);

	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Combat)
	{
		Combat->DroppedWeapon();
	}
}

void AYuanZuCharacterBase::Rebirth()
{
	YuanZuPlayerController->bDisableGameplay = false;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	AYuanZuGameMode* YuanZuGameMode = GetWorld()->GetAuthGameMode<AYuanZuGameMode>();
	if (YuanZuGameMode)
	{
		YuanZuGameMode->RequestRespawn(this, Controller);
	}
}

void AYuanZuCharacterBase::PollInit()
{
	if (!YuanZuPlayerState)
	{
		YuanZuPlayerState = GetPlayerState<AYuanZuPlayerState>();
		if (YuanZuPlayerState)
		{
			YuanZuPlayerState->AddToKillScore(0.f);
			YuanZuPlayerState->AddToDeathScore(0);
		}
	}
}

void AYuanZuCharacterBase::Move(const FInputActionValue& Value)
{
	FVector2D MoveVector = Value.Get<FVector2D>();

	if (Controller)
	{
		if (IsSwimming())
		{
			FVector SwimForwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());
			AddMovementInput(SwimForwardVector, MoveVector.Size());

		}
		else
		{
			const FRotator Rotation(0, GetControlRotation().Yaw, 0);

			const FVector ForwardVector = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
			const FVector RightVector = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

			AddMovementInput(ForwardVector, MoveVector.Y);
			AddMovementInput(RightVector, MoveVector.X);
		}
	}

}

void AYuanZuCharacterBase::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
	}
}

void AYuanZuCharacterBase::SetOverlappingWeapon(AYuanZuWeapon* Weapon)
{
	if (!bIsDeath)
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(false);
		}
		OverlappingWeapon = Weapon;
		if (IsLocallyControlled())
		{
			if (OverlappingWeapon)
			{
				OverlappingWeapon->ShowPickUpWidget(true);
			}
		}
	}
}

void AYuanZuCharacterBase::SetOVerlappingItem(AYuanZuPickupItem* Item)
{

	if (OverlappingItem)
	{
		OverlappingItem->ShowItemWidget(false);
	}
	OverlappingItem = Item;
	if(IsLocallyControlled())
	{
		if (OverlappingItem)
		{
			OverlappingItem->ShowItemWidget(true);
		}
	}

}

bool AYuanZuCharacterBase::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AYuanZuCharacterBase::GetIsAiming()
{
	return (Combat && Combat->bAiming);
}

void AYuanZuCharacterBase::StartWalk()
{
	bIsWalking = true;

	if (WalkingTimeline && bIsWalking)
	{
		WalkingTimeline->Play();
	}

	if (!HasAuthority())
	{
		ServerPlayWalkTimeline();
	}

}

void AYuanZuCharacterBase::StopWalk()
{
	bIsWalking = false;

	if (WalkingTimeline && !bIsWalking)
	{
		WalkingTimeline->Reverse();
	}

	if (!HasAuthority())
	{
		ServerReverseWalkTimeline();
	}
}

bool AYuanZuCharacterBase::IsSwimming()
{
	return (GetCharacterMovement()->IsSwimming());
}

void AYuanZuCharacterBase::SwimState()
{
	LaunchCharacter(GetActorUpVector() * 2.f, false, false);
	GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = true;
}

void AYuanZuCharacterBase::EndSwimState()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = false;
}

void AYuanZuCharacterBase::StartSwim()
{
	if (SwimTimeline)
	{
		SwimTimeline->Play();
	}
	if (!HasAuthority())
	{
		ServerPlaySwimTimeline();
	}
	else
	{
		bIsSwimming = true;
	}

}

void AYuanZuCharacterBase::StopSwim()
{
	if (SwimTimeline)
	{
		SwimTimeline->Reverse();
	}
	if (!HasAuthority())
	{
		ServerReverseSwimTimeline();
	}
	else
	{
		bIsSwimming = false;
	}

}

void AYuanZuCharacterBase::FireButtonPressed()
{
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SLD &&
			Combat->bCanThrow &&
			Combat->EquippedWeapon->GetAmmo() > 0)
		{
			Combat->StartLift();
		}
		else if(Combat->EquippedWeapon->GetWeaponType() != EWeaponType::EWT_SLD)
		{
			Combat->StartFire();
		}
	}
	else if(Combat)
	{
		Combat->StartAttack();
	}
}

void AYuanZuCharacterBase::FireButtonReleased()
{
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SLD && GetIsLift() && Combat->EquippedWeapon->GetAmmo() > 0)
		{
			Combat->SetThrow(true);
		}
		else
		{
			Combat->StopFire();
		}

	}
}

FName AYuanZuCharacterBase::SelectFireMontageName(bool bAimming)
{
	if (bIsCrouched && bAimming)
	{
		return FName("AimCrouchFire");
	}
	else if (bIsCrouched)
	{
		return FName("CrouchFire");
	}
	else if (bAimming)
	{
		return FName("AimFire");
	}
	else
	{
		return FName("Fire");
	}
	return FName();
}

FName AYuanZuCharacterBase::SelectThrowMontageName(bool bHighThrow)
{
	if (bIsCrouched)
	{
		if (bHighThrow)
		{
			UE_LOG(LogYuanZu, Warning, TEXT("Crouch_H"));
			return FName("Crouch_H");
		}
		else
		{
			UE_LOG(LogYuanZu, Warning, TEXT("Crouch_L"));
			return FName("Crouch_L");
		}
	}
	else
	{
		if (bHighThrow)
		{
			UE_LOG(LogYuanZu, Warning, TEXT("Stand_H"));
			return FName("Stand_H");
		}
		else
		{
			UE_LOG(LogYuanZu, Warning, TEXT("Stand_L"));
			return FName("Stand_L");
		}
	}
	return FName();
}

FName AYuanZuCharacterBase::SelectPullBoltMontageName()
{
	if (bIsCrouched)
	{
		return FName("Crouch");
	}
	else
	{
		return FName("Stand");
	}
	return FName();
}

void AYuanZuCharacterBase::PlayFireMontage(bool bAimming)
{
	if (Combat == nullptr || !Combat->EquippedWeapon || FireWeaponMontage.IsEmpty())return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)return;

	//将查询到的Key的动画蒙太奇返回给Montage
	UAnimMontage* Montage = FireWeaponMontage.FindRef(GetWeaponType());
	if (!Montage)return;

	AnimInstance->Montage_Play(Montage);
	AnimInstance->Montage_JumpToSection(SelectFireMontageName(bAimming), Montage);
}

void AYuanZuCharacterBase::PlayPullBoltMontage()
{
	if (Combat == nullptr || !Combat->EquippedWeapon || !PullBoltMontage)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)return;

	AnimInstance->Montage_Play(PullBoltMontage);
	AnimInstance->Montage_JumpToSection(SelectPullBoltMontageName(), PullBoltMontage);
}

void AYuanZuCharacterBase::PlayAntitankFireMontage(bool bHighThrow)
{
	if (Combat == nullptr || !Combat->EquippedWeapon || FireWeaponMontage.IsEmpty())return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)return;

	//将查询到的Key的动画蒙太奇返回给Montage
	UAnimMontage* Montage = FireWeaponMontage.FindRef(GetWeaponType());
	if (!Montage)return;

	AnimInstance->Montage_Play(Montage);
	AnimInstance->Montage_JumpToSection(SelectThrowMontageName(bHighThrow), Montage);
}

void AYuanZuCharacterBase::PlayReloadMontage(bool bAimming)
{
	if (!Combat || !Combat->EquippedWeapon || ReloadMontage.IsEmpty() || !LDQReloadMontage)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)return;
	
	EAmmoType AT = Combat->EquippedWeapon->GetAmmoType();

	//将查询到的Key的动画蒙太奇返回给Montage
	UAnimMontage* Montage = ReloadMontage.FindRef(GetWeaponType());
	if (!Montage)return;

	if (AT == EAmmoType::EAT_40MM)
	{
		AnimInstance->Montage_Play(LDQReloadMontage);
		AnimInstance->Montage_JumpToSection(SelectReloadMontageName(bAimming), LDQReloadMontage);
	}
	else
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SelectReloadMontageName(bAimming), Montage);
	}
	
}

FName AYuanZuCharacterBase::SelectReloadMontageName(bool bAimming)
{
	if (bIsCrouched)
	{
		UE_LOG(LogYuanZu, Warning, TEXT("Play EquiReload"));
		return FName("CrouchReload");
	}
	else if (Combat->bFire)
	{
		UE_LOG(LogYuanZu, Warning, TEXT("Play EquiReload"));
		return FName("FireReload");
	}
	else if (bAimming)
	{
		UE_LOG(LogYuanZu, Warning, TEXT("Play EquiReload"));
		return FName("AimReload");
	}
	else
	{
		UE_LOG(LogYuanZu, Warning, TEXT("Play EquipReload"));
		return FName("EquipReload");
	}
}

FName AYuanZuCharacterBase::SelectAttackMontageName()
{
	for (int32 i = 0; i <= AttackName.Num(); i++)
	{
		if (AttackName.IsValidIndex(i))
		{
			if (GetCharacterMovement()->IsFalling())
			{
				int32 JumpAttack = FMath::RandRange(2, 3);
				return AttackName[JumpAttack];
			}
			else
			{
				int32 Attack = FMath::RandRange(0, 1);
				return AttackName[Attack];
			}
		}
		else
		{
			return FName();
		}
	}
	return FName();
}

void AYuanZuCharacterBase::PlayAttackMontage()
{
	if (!Combat || Combat->EquippedWeapon || !AttackMontage)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (!AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			AnimInstance->Montage_Play(AttackMontage);
			AnimInstance->Montage_JumpToSection(SelectAttackMontageName(), AttackMontage);
		}
	}
}

bool AYuanZuCharacterBase::PerformUnarmedAttackSweep()
{
	if (!HasAuthority() || !GetWorld())
	{
		//如果不是服务器执行就返回
		return false;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(FName(TEXT("UnarmedAttackSweep")), false, this);

	//射线起点
	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	//射线终点
	const FVector End = Start + GetActorForwardVector() * AttackTraceDistance;
	//球形检测
	const FCollisionShape AttackShape = FCollisionShape::MakeSphere(AttackTraceRadius);//攻击形态

	//检查是否打中了
	const bool bHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Pawn, AttackShape, QueryParams);

	AActor* HitActor = HitResult.GetActor();
	if (!bHit || !HitActor || HitActor == this)
	{
		//没打中和打中自己则无效
		return false;
	}

	ApplyUnarmedAttackDamage(HitActor);
	return true;
}

//施加徒手攻击伤害
void AYuanZuCharacterBase::ApplyUnarmedAttackDamage(AActor* OtherActor)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this)
	{
		//如果不是服务器执行、没打到人、攻击到的人是自己就返回
		return;
	}

	AController* OwnerController = GetController();
	if (OwnerController == nullptr)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, OwnerController, this, UDamageType::StaticClass());

	AYuanZuCharacterBase* Character = Cast<AYuanZuCharacterBase>(OtherActor);
	if (Character)
	{
		Character->MulticastHit();
	}
}

void AYuanZuCharacterBase::SetAttackVaild(bool bIsVaild)
{
	if (!HasAuthority()) return;

	if (bIsVaild)
	{
		UE_LOG(LogYuanZu, Warning, TEXT("打人有效"));
		if (!bAttackDamageApplied)
		{
			//打中人且打中的不是自己
			bAttackDamageApplied = PerformUnarmedAttackSweep();
		}
	}
	else
	{
		UE_LOG(LogYuanZu, Warning, TEXT("打人无效"));
		bAttackDamageApplied = false;
	}
}

void AYuanZuCharacterBase::SetCharacterMaterial(ETeamType InTeamType)
{
	if (!GetMesh() || TeamMaterial.IsEmpty())return;

	const FTeamMaterial* FoundTeamMaterial = TeamMaterial.Find(InTeamType);
	if (!FoundTeamMaterial) return;

	const int32 MaterialSlots[] = { 1, 2, 3, 7 };

	//如果材质的数量小于材质插槽的数量就返回
	if (FoundTeamMaterial->Materials.Num() < UE_ARRAY_COUNT(MaterialSlots)) return;

	//i的循环次数小于插槽数量
	for (int32 i = 0; i < UE_ARRAY_COUNT(MaterialSlots); ++i)
	{
		//检查材质的i索引是否有效
		if (FoundTeamMaterial->Materials[i])
		{
			// i = 0 就是MaterialSlots的第一个元素 以此类推
			GetMesh()->SetMaterial(MaterialSlots[i], FoundTeamMaterial->Materials[i]);
		}
	}
}

void AYuanZuCharacterBase::SetJJBQAim(AYuanZuWeapon* InWeapon, bool bIsAimming)
{
	if (!InWeapon || InWeapon->GetWeaponType() != EWeaponType::EWT_JJBQ)return;

	AYuanZuSnipeProjectileWeapon* SnipeProjectileWeapon = Cast<AYuanZuSnipeProjectileWeapon>(InWeapon);
	if (SnipeProjectileWeapon)
	{
		SnipeProjectileWeapon->OpenScope(bIsAimming);
	}
}

void AYuanZuCharacterBase::SetAmmoType(AYuanZuWeapon* InWeapon)
{
	if (!InWeapon)return;

	AmmoMesh->SetStaticMesh(InWeapon->AmmoTypeMesh);
}

void AYuanZuCharacterBase::StopFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && !FireWeaponMontage.IsEmpty())
	{
		UAnimMontage* Montage = FireWeaponMontage.FindRef(GetWeaponType());
		if (!Montage)return;
		AnimInstance->Montage_Stop(0.1f, Montage);
	}
}

bool AYuanZuCharacterBase::GetIsFire()
{
	return Combat && Combat->bFire;
}

void AYuanZuCharacterBase::PlayHitReactMontage()
{
	if (!Combat || Combat->EquippedWeapon || !HitReactMontage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		UE_LOG(LogYuanZu, Warning, TEXT("HitReact"));
	}
}

void AYuanZuCharacterBase::OnRep_CurrentHealth()
{
	UpdateHealth();
}

void AYuanZuCharacterBase::OnRep_OverlappingItem(AYuanZuPickupItem* LastWeapon)
{
	if (LastWeapon)
	{
		LastWeapon->ShowItemWidget(false);
	}

	if (OverlappingItem && IsLocallyControlled())
	{
		OverlappingItem->ShowItemWidget(true);
	}
}

void AYuanZuCharacterBase::UpdateHealth()
{
	YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(GetController()) : YuanZuPlayerController;
	if (YuanZuPlayerController)
	{
		YuanZuHUD = Cast<AYuanZuHUD>(YuanZuPlayerController->GetHUD());
		YuanZuPlayerController->SetHealth(CurrentHealth, MaxHealth);
	}
}

void AYuanZuCharacterBase::OnSwimTimelineUpdate(float Alpha)
{
	if (SwimRetardance && GetCharacterMovement())
	{
		//计算目标速度
		float TargetSpeed = FMath::Lerp(0.f, 300.f, Alpha);
		GetCharacterMovement()->MaxSwimSpeed = TargetSpeed;
	}
}

void AYuanZuCharacterBase::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

AYuanZuWeapon* AYuanZuCharacterBase::GetEquippedWeapon()const
{
	if (Combat == nullptr)
	{
		return nullptr;
	}

	return Combat->EquippedWeapon;
}

FVector AYuanZuCharacterBase::GetHitTarget() const
{
	if (Combat)
	{
		return Combat->GetHitTarget();
	}
	return FVector::ZeroVector;
}

EWeaponType AYuanZuCharacterBase::GetWeaponType()const
{
	if (Combat && Combat->EquippedWeapon)
	{
		return Combat->EquippedWeapon->GetWeaponType();
	}
	return EWeaponType();
}

bool AYuanZuCharacterBase::GetIsLift()const
{
	if (Combat && Combat->EquippedWeapon)
	{
		return Combat->GetIsLift();
	}
	return false;
}

bool AYuanZuCharacterBase::GetIsHighThrow() const
{
	if (Combat && Combat->EquippedWeapon)
	{
		return Combat->GetIsHighThrow();
	}
	return false;
}

void AYuanZuCharacterBase::UpdateAmmo()
{
	if (!Combat) return;

	Combat->UpdateAmmoCount();
}

void AYuanZuCharacterBase::ShowAmmoMesh(bool bIsVisible)
{
	AmmoMesh->SetVisibility(bIsVisible);
}
