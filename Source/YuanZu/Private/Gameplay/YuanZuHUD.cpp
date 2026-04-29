// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/YuanZuHUD.h"
#include "Engine/Texture2D.h"
#include "UI/YuanZuCharacterOverlay.h"
#include "Gameplay/YuanZuPlayerController.h"
#include "UI/YuanZuGameRecord.h"
#include "UI/YuanZuMainUIBase.h"
#include "Character/YuanZuCharacterBase.h"
#include "UI/YuanZuAnnouncement.h"

void AYuanZuHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}

void AYuanZuHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AYuanZuHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor LinearColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, LinearColor);
}

void AYuanZuHUD::CreateMainUI()
{
	if (MainUIBaseClass)
	{
		AYuanZuPlayerController* PC = Cast<AYuanZuPlayerController>(GetOwningPlayerController());
		if (PC)
		{
			MainUIBase = CreateWidget<UYuanZuMainUIBase>(PC, MainUIBaseClass);
			if (MainUIBase)
			{
				MainUIBase->AddToViewport();

			};
		}
	}
}

void AYuanZuHUD::AddAnnouncement()
{
	// 如果Announcement已经存在，不需要重复创建
	if (Announcement)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();

	if (PC && AnnouncementClass)
	{
		Announcement = CreateWidget<UYuanZuAnnouncement>(PC, AnnouncementClass);
		if (Announcement)
		{
			Announcement->AddToViewport();
		}
	}
}
