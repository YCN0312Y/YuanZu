// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/YuanZuPlayerRecord.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UYuanZuPlayerRecord::InitPlayerRecord(ETeamType& InTeamType, FString& InPlayerName, float& InKillScore, int32& InDeathScore, int32& InSAScore)
{
	if (InPlayerName.IsEmpty() || !RedIcon || !BlueIcon)return;

	if (PlayerImage)
	{
		switch (InTeamType)
		{
		case ETeamType::ETT_Red:
			PlayerImage->SetBrushFromTexture(RedIcon);
			break;
		case ETeamType::ETT_Blue:
			PlayerImage->SetBrushFromTexture(BlueIcon);
			break;
		}
	}

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InPlayerName));
	}
	if (KillScoreText)
	{
		KillScoreText->SetText(FText::AsNumber(FMath::FloorToInt32(InKillScore)));
	}
	if (DeathScoreText)
	{
		DeathScoreText->SetText(FText::AsNumber(InDeathScore));
	}
	if (SecondaryScoreText)
	{
		SecondaryScoreText->SetText(FText::AsNumber(InSAScore));
	}
}
