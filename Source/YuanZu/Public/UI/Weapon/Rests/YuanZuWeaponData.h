#pragma once


#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Weapons/Rests/YuanZuWeaponTypes.h"
#include "YuanZuWeaponData.generated.h"

USTRUCT(BlueprintType)
struct FYuanZuWeaponData
{
	GENERATED_BODY()
public:
	//武器类型
	UPROPERTY(EditAnywhere, Category = "Weapon")
	EAmmoType AmmoType;
	//武器类型
	UPROPERTY(EditAnywhere, Category = "Weapon")
	FText AmmoText;
	//武器图标
	UPROPERTY(EditAnywhere, Category = "Weapon")
	UTexture2D* WeaponTexture;
};