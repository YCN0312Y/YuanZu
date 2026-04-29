// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/YuanZuCharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UYuanZuCharacterOverlay::SetPlayerName(const FString& PlayerName)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(PlayerName));
	}
}

void UYuanZuCharacterOverlay::SetPlayerIcon(ETeamType InTeamType)
{
	if (PlayerIcon)
	{
		switch (InTeamType)
		{
		case ETeamType::ETT_Red:
			PlayerIcon->SetBrushFromTexture(RedIcon);
			break;

		case ETeamType::ETT_Blue:
			PlayerIcon->SetBrushFromTexture(BlueIcon);
			break;

		case ETeamType::ETT_None:
		default:
			PlayerIcon->SetBrushFromTexture(nullptr);
			break;
		}
	}
}
