// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Weapon/YuanZuWeaponProperty.h"
#include "Weapons/YuanZuWeapon.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "UI/Weapon/Rests/YuanZuWeaponData.h"

void UYuanZuWeaponProperty::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UYuanZuWeaponProperty::NativeConstruct()
{
	Super::NativeConstruct();
}

void UYuanZuWeaponProperty::SetWeaponName(const FText& InWeaponName)
{
	if (WeaponNameText)
	{
		WeaponNameText->SetText(InWeaponName);
	}
}

void UYuanZuWeaponProperty::SetWeaponNameByWeapon(AYuanZuWeapon* InWeapon)
{
	if (!WeaponNameText)return;

	if (InWeapon)
	{
		for (int32 i = 0; i < WeaponData.Num(); i++)
		{
			if (WeaponData.IsValidIndex(i))
			{
				for (FYuanZuWeaponData& Data : WeaponData)
				{
					if (InWeapon->GetAmmoType() == Data.AmmoType)
					{
						WeaponImage->SetBrushFromTexture(Data.WeaponTexture);
						AmmoTypeText->SetText(Data.AmmoText);
					}
				}
			}
		}
		WeaponNameText->SetText(InWeapon->GetWeaponDisplayName());
	}
	else
	{
		WeaponNameText->SetText(FText::GetEmpty());
		if (WeaponImage)
		{
			WeaponImage->SetBrushFromTexture(nullptr);
		}
	}


}
