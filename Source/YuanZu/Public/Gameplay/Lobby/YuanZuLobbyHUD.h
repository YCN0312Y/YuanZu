// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "YuanZuLobbyHUD.generated.h"

class UYuanZuLobby;

UCLASS()
class YUANZU_API AYuanZuLobbyHUD : public AHUD
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

public:
	//´óÌüÀà
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuLobby> LobbyClass;
	//´óÌü
	UPROPERTY()
	UYuanZuLobby* Lobby;
	
	void CreateLobbyWidget();
};
