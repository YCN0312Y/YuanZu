// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Weapon/YuanZuPickUp.h"
#include "Weapons/YuanZuWeapon.h"
#include "Components/TextBlock.h"
#include "Items/YuanZuPickupItem.h"

void UYuanZuPickUp::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemsText)
	{
		ItemsText->TextDelegate.BindDynamic(this, &UYuanZuPickUp::GetItemText);
		//同步属性
		ItemsText->SynchronizeProperties();
	}
}

void UYuanZuPickUp::SetWeapon(AYuanZuWeapon* InWeapon)
{
	 Weapon = InWeapon;

	 if (ItemsText)
	 {
		 ItemsText->SynchronizeProperties();
	 }
}

void UYuanZuPickUp::SetItem(AYuanZuPickupItem* InItem)
{
	Item = InItem;
	if (ItemsText)
	{
		ItemsText->SynchronizeProperties();
	}
}

FText UYuanZuPickUp::GetItemText()
{
	if (Weapon)
	{
		return Weapon->WeaponName;
	}
	if (Item)
	{
		return Item->ItemName;
	}
	return FText::GetEmpty();
}