// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiplayerPlayerController.generated.h"

class UPlayerInformation;

UCLASS()
class MULTIPLAYERSESSIONS_API AMultiplayerPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

public:
	//菜单类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPlayerInformation> PlayerInformationClass;
	//比赛类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FString TypeToMatch = TEXT("FreeForAll");
	//大厅地图
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FString HallPath = TEXT("");
	//菜单
	UPROPERTY()
	UPlayerInformation* PlayerInformation;
};
