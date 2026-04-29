#pragma once

#include "CoreMinimal.h"
#include "YuanZuLobbyHintInfo.generated.h"

USTRUCT(BlueprintType)
struct FYuanZuLobbyHintInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	FString StateText;

	UPROPERTY()
	FString TimeString;
};
