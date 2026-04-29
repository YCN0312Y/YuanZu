// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/Rests/YuanZuTurningInPlace.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "YuanZuAnimInstance.generated.h"

class AYuanZuCharacterBase;
class AYuanZuWeapon;

UCLASS()
class YUANZU_API UYuanZuAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	//初始化
	virtual void NativeInitializeAnimation() override;
	//每帧都调用
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	AYuanZuCharacterBase* YuanZuCharacter;
	//速度
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	float Speed;
	//是否在空中
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	//是否在加速
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
	//是否装备武器
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsWeaponEquipped;
	//是否蹲下
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;
	//是否瞄准
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;
	//角色旋转角度
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	float Direction;
	//角色瞄准AO_Yaw
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;
	//角色瞄准AO_Pitch
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;
	//左手变换
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;
	//角色游泳状态
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsSwimState;
	//角色游泳速度
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	float SwimmingSpeed;
	//是否开火
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsFire;
	//是否死亡
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	bool bIsDeath;
	//移动状态
	UPROPERTY(BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	int32 MoveState;
	//旋转角度
	UPROPERTY(BlueprintReadOnly, Category = "YuanZu|Movement", meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;
	//右手旋转
	UPROPERTY(BlueprintReadOnly, Category = "YuanZu|Movement", meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;
	//武器类型
	UPROPERTY(BlueprintReadOnly, Category = "YuanZu|Movement", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	//装备的武器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	AYuanZuWeapon* EquippedWeapon;

	FRotator DeltaRotation;

	bool bIsDoOnce = false;

private:
	void DoOnce();
	void ResetDoOnce();

public:
	FORCEINLINE bool GetIsIAir() { return bIsInAir; };

};
