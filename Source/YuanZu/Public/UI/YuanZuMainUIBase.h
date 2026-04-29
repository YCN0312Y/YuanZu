// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuMainUIBase.generated.h"

class UTextBlock;
class UYuanZuCharacterOverlay;
class UYuanZuGameRecord;
class UYuanZuWeaponProperty;

UCLASS()
class YUANZU_API UYuanZuMainUIBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameTimeText;
	//玩家信息
	UPROPERTY(meta = (BindWidget))
	UYuanZuCharacterOverlay* CharacterOverlay;
	//游戏战绩
	UPROPERTY()
	UYuanZuGameRecord* GameRecord;
	//游戏战绩类
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuGameRecord> GameRecordClass;
	//武器
	UPROPERTY(meta = (BindWidget))
	UYuanZuWeaponProperty* WeaponProperty;

public:
	template<typename T>
	void CreateWidgetAndAddToViewport(APlayerController* PC, T*& Widget, TSubclassOf<T>WidgetClass)
	{
		if (!PC && WidgetClass) return;

		Widget = CreateWidget<T>(PC, WidgetClass);
		if (Widget)
		{
			Widget->AddToViewport();
		}

	}

public:
	//添加战绩
	void ShowGameRecord(bool bVisible);
	void ShowWeaponProperty(bool bVisible);
};
