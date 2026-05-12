#pragma once

UENUM(BlueprintType)
enum class EYuanZuCombatState : uint8
{
	ECS_UnEquip UMETA(DisplayName = "UnEquip"),
	ECS_Equip UMETA(DisplayName = "Equip"),
	ECS_Reload UMETA(DisplayName = "Reload"),
	ECS_Antitank UMETA(DisplayName = "Antitank"),

	ECS_Max UMETA(DisplayName = "Max"),

};