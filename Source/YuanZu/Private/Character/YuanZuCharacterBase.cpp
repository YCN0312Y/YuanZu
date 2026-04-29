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

AYuanZuCharacterBase::AYuanZuCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	//弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 350.0f;
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

	bUseControllerRotationYaw = false;
	//生成碰撞处理方法为：尝试调整位置 但始终会重生
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//头顶小部件
	//OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	//OverHeadWidget->SetupAttachment(RootComponent);

	//战斗组件
	Combat = CreateDefaultSubobject<UYuanZuCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	//时间轴
	RunningTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RunningTimeline"));
	SwimTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SwimTimeline"));

	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;//方向转至移动角度
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
		SetCharacterMaterila(YuanZuPS->GetTeamType());
	}
}

void AYuanZuCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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
	DOREPLIFETIME(AYuanZuCharacterBase, bIsRunning);
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

void AYuanZuCharacterBase::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);

	if (!CameraBoom)return;

	TargetCameraLocation = CrouchCameraLocation;
}

void AYuanZuCharacterBase::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);

	if (!CameraBoom)return;

	TargetCameraLocation = StandCameraLocation;
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
		TargetCameraLocation = StandCameraLocation;
	}

	if (RunningRetardance && RunningTimeline)
	{
		OnWalkRunTimelineUpdateDelegate.BindUFunction(this, FName("OnWalkRunTimelineUpdate"));
		RunningTimeline->AddInterpFloat(RunningRetardance, OnWalkRunTimelineUpdateDelegate);
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
		SetCharacterMaterila(YuanZuPS->GetTeamType());
	}
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


void AYuanZuCharacterBase::OnWalkRunTimelineUpdate(float Alpha)
{
	if (RunningRetardance && GetCharacterMovement() && !bIsCrouched)
	{
		if (Combat->EquippedWeapon)
		{
			int32 DefaultSpeed;
			int32 EquipHJTSpeed;

			switch (GetWeaponType())
			{
			default:
				DefaultSpeed = FMath::Lerp(300.0f, 600.0f, Alpha);
				GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
				UE_LOG(LogYuanZu, Warning, TEXT("EquipTJBQSpeed[%d]"), DefaultSpeed);
				break;
			case EWeaponType::EWT_HJT:
				EquipHJTSpeed = FMath::Lerp(300.0f, 450.0f, Alpha);
				GetCharacterMovement()->MaxWalkSpeed = EquipHJTSpeed;
				UE_LOG(LogYuanZu, Warning, TEXT("EquipHJTSpeed[%d]"), EquipHJTSpeed);
				break;
			}
		}
		else
		{
			//计算目标速度
			float TargetSpeed = FMath::Lerp(300.0f, 600.0f, Alpha);
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

		bUseControllerRotationYaw = true;//使用控制器旋转
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)//跑或跳时
	{
		ContinuousYaw = 0.f;
		LastYaw = AimYaw;

		StartAimRotation = FRotator(0.f, AimYaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;//使用控制器旋转
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

void AYuanZuCharacterBase::ServerPlayRunTimeline_Implementation()
{
	if (RunningTimeline)
	{
		RunningTimeline->Play();

		UE_LOG(LogYuanZu, Warning, TEXT("Running"));
	}
}

void AYuanZuCharacterBase::ServerReverseRunTimeline_Implementation()
{
	if (RunningTimeline)
	{
		RunningTimeline->Reverse();
	}
}

void AYuanZuCharacterBase::OnRep_IsRunning()
{
	if (RunningTimeline)
	{
		if (bIsRunning)
		{
			RunningTimeline->Play();
		}
		else
		{
			RunningTimeline->Reverse();
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
			bUseControllerRotationYaw = true;//使用控制器旋转
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

bool AYuanZuCharacterBase::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AYuanZuCharacterBase::GetIsAiming()
{
	return (Combat && Combat->bAiming);
}

void AYuanZuCharacterBase::StartRun()
{
	bIsRunning = true;

	if (RunningTimeline && bIsRunning)
	{
		RunningTimeline->Play();
	}

	if (!HasAuthority())
	{
		ServerPlayRunTimeline();
	}

}

void AYuanZuCharacterBase::StopRun()
{
	bIsRunning = false;

	if (RunningTimeline && !bIsRunning)
	{
		RunningTimeline->Reverse();
	}

	if (!HasAuthority())
	{
		ServerReverseRunTimeline();
	}
}

bool AYuanZuCharacterBase::IsSwimming()
{
	return (GetCharacterMovement()->IsSwimming());
}

void AYuanZuCharacterBase::SwimState()
{
	bUseControllerRotationYaw = true;

	LaunchCharacter(GetActorUpVector() * 2.f, false, false);
	GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = true;
}

void AYuanZuCharacterBase::EndSwimState()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = false;

	if (Combat->EquippedWeapon)
	{
		bUseControllerRotationYaw = true;
	}
	else
	{
		bUseControllerRotationYaw = false;
	}
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
		Combat->StartFire();
	}
}

void AYuanZuCharacterBase::FireButtonReleased()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->StopFire();
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

void AYuanZuCharacterBase::PlayFireMontage(bool bAimming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr || FireWeaponMontage.IsEmpty())return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		//遍历换弹蒙太奇的索引是否有效
		for (int32 i = 0; i < FireWeaponMontage.Num(); i++)
		{
			if (FireWeaponMontage.IsValidIndex(i))
			{
				UE_LOG(LogYuanZu, Warning, TEXT("FireWeaponMontage [%d] Index Valid!"), i);
			}
		}

		switch (GetWeaponType())
		{
		case EWeaponType::EWT_TJBQ:
			AnimInstance->Montage_Play(FireWeaponMontage[0]);
			AnimInstance->Montage_JumpToSection(SelectFireMontageName(bAimming), FireWeaponMontage[0]);
			break;
		case EWeaponType::EWT_HJT:
			AnimInstance->Montage_Play(FireWeaponMontage[1]);
			AnimInstance->Montage_JumpToSection(SelectFireMontageName(), FireWeaponMontage[1]);
			break;
		case EWeaponType::EWT_SQ:
			AnimInstance->Montage_Play(FireWeaponMontage[2]);
			AnimInstance->Montage_JumpToSection(SelectFireMontageName(), FireWeaponMontage[2]);
			break;
		case EWeaponType::EWT_JJBQ:
			AnimInstance->Montage_Play(FireWeaponMontage[3]);
			AnimInstance->Montage_JumpToSection(SelectFireMontageName(), FireWeaponMontage[3]);
			break;
		}
	}
}

void AYuanZuCharacterBase::PlayReloadMontage()
{
	if (!Combat || !Combat->EquippedWeapon || ReloadMontage.IsEmpty())return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		//遍历换弹蒙太奇的索引是否有效
		for (int32 i = 0; i < ReloadMontage.Num(); i++)
		{
			if (ReloadMontage.IsValidIndex(i))
			{
				UE_LOG(LogYuanZu, Warning, TEXT("ReloadMontage [%d] Index Valid!"), i);
			}
		}

		switch (GetWeaponType())
		{
		case EWeaponType::EWT_TJBQ:
			AnimInstance->Montage_Play(ReloadMontage[0]);
			AnimInstance->Montage_JumpToSection(SelectReloadMontageName(), ReloadMontage[0]);
			break;
		case EWeaponType::EWT_HJT:
			AnimInstance->Montage_Play(ReloadMontage[1]);
			AnimInstance->Montage_JumpToSection(SelectReloadMontageName(), ReloadMontage[1]);
			break;
		case EWeaponType::EWT_SQ:
			AnimInstance->Montage_Play(ReloadMontage[2]);
			AnimInstance->Montage_JumpToSection(SelectReloadMontageName(), ReloadMontage[2]);
			break;
		case EWeaponType::EWT_JJBQ:
			AnimInstance->Montage_Play(ReloadMontage[3]);
			AnimInstance->Montage_JumpToSection(SelectReloadMontageName(), ReloadMontage[3]);
			break;
		}
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

void AYuanZuCharacterBase::SetCameraLocation(float DeltaTime)
{
	if (!CameraBoom) return;

	FVector Offset = CameraBoom->GetRelativeLocation();
	Offset.Z = FMath::FInterpTo(Offset.Z, TargetCameraLocation, DeltaTime, CrouchInterpSpeed);
	CameraBoom->SetRelativeLocation(Offset);
}

void AYuanZuCharacterBase::SetCharacterMaterila(ETeamType InTeamType)
{
	if (!GetMesh() || TeamCharacterMaterial.Num() < 8)return;

		switch (InTeamType)
		{
		case ETeamType::ETT_Red:
			GetMesh()->SetMaterial(1, TeamCharacterMaterial[0]);
			GetMesh()->SetMaterial(2, TeamCharacterMaterial[1]);
			GetMesh()->SetMaterial(3, TeamCharacterMaterial[2]);
			GetMesh()->SetMaterial(7, TeamCharacterMaterial[3]);
			UE_LOG(LogYuanZu, Warning, TEXT("ETT_Red"));
			break;
		case ETeamType::ETT_Blue:
			GetMesh()->SetMaterial(1, TeamCharacterMaterial[4]);
			GetMesh()->SetMaterial(2, TeamCharacterMaterial[5]);
			GetMesh()->SetMaterial(3, TeamCharacterMaterial[6]);
			GetMesh()->SetMaterial(7, TeamCharacterMaterial[7]);
			UE_LOG(LogYuanZu, Warning, TEXT("ETT_Red"));
			break;
		case ETeamType::ETT_None:
		default:
			break;
		}
}

void AYuanZuCharacterBase::SetAmmoType(AYuanZuWeapon* InWeapon)
{
	if (!InWeapon)return;

	switch (InWeapon->GetWeaponType())
	{
	case EWeaponType::EWT_TJBQ:
		AmmoMesh->SetStaticMesh(InWeapon->AmmoTypeMesh);
		break;
	case EWeaponType::EWT_HJT:
		AmmoMesh->SetStaticMesh(InWeapon->AmmoTypeMesh);
		break;
	case EWeaponType::EWT_SQ:
		AmmoMesh->SetStaticMesh(InWeapon->AmmoTypeMesh);
		break;
	case EWeaponType::EWT_JJBQ:
		AmmoMesh->SetStaticMesh(InWeapon->AmmoTypeMesh);
		break;
	}
}

void AYuanZuCharacterBase::StopFireMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.1f, FireWeaponMontage[0]);
	}
}

bool AYuanZuCharacterBase::GetIsFire()
{
	return Combat && Combat->bFire;
}

void AYuanZuCharacterBase::PlayHitReactMontage(bool bIsHit)
{
	if (Combat)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (HitReactMontage && AnimInstance)
		{
			if (Combat->EquippedWeapon)
			{
				AnimInstance->Montage_Play(HitReactMontage);
				AnimInstance->Montage_JumpToSection(TEXT("Front"));
				UE_LOG(LogYuanZu, Warning, TEXT("Front"));
			}
			else
			{
				AnimInstance->Montage_Play(HitReactMontage_NoWeapon);
				UE_LOG(LogYuanZu, Warning, TEXT("NoWeapon"));
			}
		}
	}
}

void AYuanZuCharacterBase::OnRep_CurrentHealth()
{
	UpdateHealth();
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

void AYuanZuCharacterBase::UpdateAmmo()
{
	if (Combat == nullptr) return;

	Combat->UpdateAmmoCount();
}

void AYuanZuCharacterBase::ShowAmmoMesh(bool bIsVisible)
{
	AmmoMesh->SetVisibility(bIsVisible);
}
