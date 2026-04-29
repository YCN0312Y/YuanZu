// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerPlayerController.h"
#include "UI/PlayerInformation.h"
#include "UI/Menu.h"

void AMultiplayerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	if (PlayerInformationClass)
	{
		PlayerInformation = CreateWidget<UPlayerInformation>(this, PlayerInformationClass);

		if (PlayerInformation)
		{
			PlayerInformation->AddToViewport();
			PlayerInformation->WBP_Menu->MenuSetup(TypeToMatch, HallPath);
		}
	}
}