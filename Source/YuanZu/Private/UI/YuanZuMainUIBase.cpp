// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/YuanZuMainUIBase.h"
#include "UI/YuanZuCharacterOverlay.h"
#include "UI/YuanZuGameRecord.h"
#include "UI/Weapon/YuanZuWeaponProperty.h"

void UYuanZuMainUIBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (GameRecord)
	{
		GameRecord->SetVisibility(ESlateVisibility::Hidden);
	}
	if (WeaponProperty)
	{
		WeaponProperty->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UYuanZuMainUIBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		CreateWidgetAndAddToViewport(PC, GameRecord, GameRecordClass);
		GameRecord->RefreshRecordList();
		GameRecord->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UYuanZuMainUIBase::NativeConstruct()
{
	Super::NativeConstruct();


}

void UYuanZuMainUIBase::ShowGameRecord(bool bVisible)
{
	if (GameRecord)
	{
		if (bVisible)
		{
			GameRecord->RefreshRecordList();
		}

		GameRecord->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UYuanZuMainUIBase::ShowWeaponProperty(bool bVisible)
{
	if (bVisible)
	{
		WeaponProperty->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		WeaponProperty->SetVisibility(ESlateVisibility::Hidden);
	}
}
