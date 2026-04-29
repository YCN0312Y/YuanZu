// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuCharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UTexture2D;

UCLASS()
class YUANZU_API UYuanZuCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//生命值进度条
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgress;
	//当前生命值
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentHealth;
	//最大生命值
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxHealth;
	//玩家名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
	//玩家头像
	UPROPERTY(meta = (BindWidget))
	UImage* PlayerIcon;

	//红方头像
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TObjectPtr<UTexture2D>RedIcon;
	//蓝方头像
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TObjectPtr<UTexture2D>BlueIcon;

public:
	//设置玩家名称
	UFUNCTION(BlueprintCallable, Category = UI)
	void SetPlayerName(const FString& PlayerName);
	void SetPlayerIcon(ETeamType InTeamType);
};
