#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_TJBQ UMETA(DisplayName = "TJBQ"),
	EWT_LDQ UMETA(DisplayName = "LDQ"),
	EWT_BS UMETA(DisplayName = "BS"),
	EWT_SQ UMETA(DisplayName = "SQ"),
	EWT_HJT UMETA(DisplayName = "HJT"),
	EWT_SDQ UMETA(DisplayName = "SDQ"),
	EWT_JJBQ UMETA(DisplayName = "JJBQ"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")

};	