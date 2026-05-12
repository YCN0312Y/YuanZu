// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/YuanZuAnimInstance.h"
#include "Character/YuanZuCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/YuanZuWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/RotationMatrix.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/YuanZuPlayerController.h"

void UYuanZuAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	YuanZuCharacter = Cast<AYuanZuCharacterBase>(TryGetPawnOwner());
}

void UYuanZuAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (YuanZuCharacter == nullptr)
	{
		YuanZuCharacter = Cast<AYuanZuCharacterBase>(TryGetPawnOwner());
	}
	if (YuanZuCharacter == nullptr)
	{
		return;
	}
	//根据是否瞄准、移动、跑步等进行更新
	FVector Velocity = YuanZuCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	//检查角色是否空中
	bIsInAir = YuanZuCharacter->GetCharacterMovement()->IsFalling();
	//角色速度 > 0 返回"真" < 0 返回"假"
	bIsAccelerating = YuanZuCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;
	bIsWeaponEquipped = YuanZuCharacter->IsWeaponEquipped();//根据角色传递过来的值决定是否拾取武器
	EquippedWeapon = YuanZuCharacter->GetEquippedWeapon();//获取是否装备武器
	bIsCrouched = YuanZuCharacter->bIsCrouched;//是否蹲下
	bIsAiming = YuanZuCharacter->GetIsAiming();//是否瞄准
	AO_Yaw = YuanZuCharacter->GetAO_Yaw();//获取AO_Yaw
	AO_Pitch = YuanZuCharacter->GetAO_Pitch();//获取AO_Pitch
	TurningInPlace = YuanZuCharacter->GetTurningInPlace();//获取旋转方向
	bIsSwimState = YuanZuCharacter->IsSwimming();//是否游泳
	SwimmingSpeed = YuanZuCharacter->GetCharacterMovement()->MaxSwimSpeed;//获取游泳速度
	bIsFire = YuanZuCharacter->GetIsFire();//是否开火
	bIsDeath = YuanZuCharacter->GetIsDeath();//是否死亡
	WeaponType = YuanZuCharacter->GetWeaponType();//装备的武器类型
	bIsLift = YuanZuCharacter->GetIsLift();//投掷物是否已扔出

	if (!bIsAccelerating)
	{
		DoOnce();
	}
	else
	{
		ResetDoOnce();
	}

	//角色移动方向
	FRotator AimRotation = YuanZuCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(YuanZuCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 5.0f);
	Direction = DeltaRotation.Yaw;

	if (bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && YuanZuCharacter->GetMesh())
	{
		// 左手 IK 目标：取武器上的 LeftHandSocket
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPosition;
		FRotator OutRotation;
		YuanZuCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), LeftHandTransform.Rotator(), OutPosition, OutRotation);

		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		LeftHandTransform.SetScale3D(FVector(1.f, 1.f, 1.f));

		if (YuanZuCharacter->IsLocallyControlled())
		{
			const FTransform RightHandTransform = YuanZuCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);

			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), YuanZuCharacter->GetHitTarget());

			const FRotator N_DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, RightHandTransform.Rotator());

			RightHandRotation = FMath::RInterpTo(RightHandRotation, N_DeltaRotation, DeltaTime, 15.f);
		}
		else
		{
			RightHandRotation = FRotator::ZeroRotator;
		}
	}

}

void UYuanZuAnimInstance::DoOnce()
{
	if (bIsDoOnce)
	{
		return;
	}
	MoveState = FMath::Floor(Speed / 310.f);
	bIsDoOnce = true;
}

void UYuanZuAnimInstance::ResetDoOnce()
{
	bIsDoOnce = false;
}
