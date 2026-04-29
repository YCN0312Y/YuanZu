#pragma once

#include "CoreMinimal.h"
#include "YuanZuMapOptionData.generated.h"

USTRUCT(BlueprintType)
struct FYuanZuMapOptionData
{
	GENERATED_BODY()

public:
	// 下拉框里显示的地图名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapDisplayName;

	// 实际切图用的地图路径
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapPath;
};