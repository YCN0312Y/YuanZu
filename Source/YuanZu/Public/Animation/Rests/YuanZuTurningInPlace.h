#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_NotTurnin UMETA(Display = "不转"),
	ETIP_Left UMETA(Display = "向左转"),
	ETIP_Right UMETA(Display = "向右转"),

	ETIP_MAX UMETA(Display = "默认最大")
};