// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuPlayerRecord.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class YUANZU_API UYuanZuPlayerRecord : public UUserWidget
{
	GENERATED_BODY()

public:
	//玩家头像
	UPROPERTY(meta = (BindWidget))
	UImage* PlayerImage;
	//玩家名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
	//击杀分数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillScoreText;
	//死亡分数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathScoreText;
	//助攻分数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SecondaryScoreText;

	//红方头像
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TObjectPtr<UTexture2D>RedIcon;
	//蓝方头像
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TObjectPtr<UTexture2D>BlueIcon;

public:
	//初始化玩家战绩
	void InitPlayerRecord(ETeamType& InTeamType, FString& InPlayerName, float& InKillScore, int32& InDeathScore, int32& InSecondaryScore);
};
