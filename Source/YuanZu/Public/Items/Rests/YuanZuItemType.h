#pragma once

UENUM(BlueprintType)
enum class EItemType: uint8
{
	EIT_Weapon UMETA(DisplayName = "Weapon"),
	EIT_Ammo UMETA(DisplayName = "Item"),
	EIT_Recover UMETA(DisplayName = "Recover"),

	EIT_Max UMETA(DiaplayName = "Max")

};