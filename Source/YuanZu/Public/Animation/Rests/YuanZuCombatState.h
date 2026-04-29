#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(Display = "Unoccupied"),
	ECS_Reloading UMETA(Display = "Reloading"),

	ECS_MAX UMETA(Display = "DefaultMAX")
};