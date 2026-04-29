// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/PlayerInformation.h"
#include "YuanZuHUD.generated.h"

class UTexture2D;

class UYuanZuMainUIBase;
class UYuanZuAnnouncement;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UTexture2D* CrosshairsCenter;//中心
	UTexture2D* CrosshairsLeft;//左
	UTexture2D* CrosshairsRight;//右
	UTexture2D* CrosshairsTop;//上
	UTexture2D* CrosshairsBottom;//下
	//准心间距
	float CrosshairSpread;
	//准心颜色
	FLinearColor CrosshairColor;

};

UCLASS()
class YUANZU_API AYuanZuHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	UPlayerInformation* PlayerInformation;
	
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

public:
	//主UI
	UPROPERTY()
	UYuanZuMainUIBase* MainUIBase;
	//主UI类
	UPROPERTY(EditAnywhere, Category = "YuanZu|PlayerState")
	TSubclassOf<UYuanZuMainUIBase> MainUIBaseClass;
	//等待比赛开始
	UPROPERTY()
	UYuanZuAnnouncement* Announcement;
	//等待比赛开始类
	UPROPERTY(EditAnywhere, Category = "YuanZu|UI")
	TSubclassOf<UYuanZuAnnouncement> AnnouncementClass;


private:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor LinearColor);

public:
	void CreateMainUI();
	void AddAnnouncement();

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; };
};
