// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/YuanZuAmmoPickup.h"
#include "Character/YuanZuCharacterBase.h"
#include "Components/YuanZuCombatComponent.h"

AYuanZuAmmoPickup::AYuanZuAmmoPickup()
{

}

void AYuanZuAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

	ItemType = EItemType::EIT_Ammo;
}

void AYuanZuAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
