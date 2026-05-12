#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_None UMETA(DisplayName = "None"),
	EWT_TJBQ UMETA(DisplayName = "TJBQ"),
	EWT_LDQ UMETA(DisplayName = "LDQ"),
	EWT_BS UMETA(DisplayName = "BS"),
	EWT_SQ UMETA(DisplayName = "SQ"),
	EWT_HJT UMETA(DisplayName = "HJT"),
	EWT_JJBQ UMETA(DisplayName = "JJBQ"),
	EWT_SLD UMETA(DisplayName = "SLD"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")

};

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_762 UMETA(DisplayName = "ZDBQ Ammo"),
	EAT_556 UMETA(DisplayName = "TJBQ Ammo"),
	EAT_9mm UMETA(DisplayName = "SQ Ammo"),
	EAT_2KJ UMETA(DisplayName = "HJT Ammo"),
	EAT_425 UMETA(DisplayName = "JJBQ Ammo"),
	EAT_40MM UMETA(DisplayName = "LDQ Ammo"),
	EAT_G67 UMETA(DisplayName = "SLD Ammo"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")

};

UENUM(BlueprintType)
enum class EWeaponSlot :uint8
{
	EWS_Primary UMETA(DisplayName = "Primary1"),
	EWS_Pistol UMETA(DisplayName = "Pistol"),
	EWS_Melee UMETA(DisplayName = "Melee"),
	EWS_Throwable UMETA(DisplayName = "Throwable"),

	EWS_Max UMETA(DisplayName = "Max"),
};
