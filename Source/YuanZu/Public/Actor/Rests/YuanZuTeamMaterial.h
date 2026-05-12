#pragma once

#include "CoreMinimal.h"
#include "YuanZuTeamMaterial.generated.h"

USTRUCT(BlueprintType)
struct FTeamMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<UMaterialInterface*> Materials;
};