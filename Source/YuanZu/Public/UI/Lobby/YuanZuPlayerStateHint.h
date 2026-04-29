// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuPlayerStateHint.generated.h"

class UTextBlock;

UCLASS()
class YUANZU_API UYuanZuPlayerStateHint : public UUserWidget
{
	GENERATED_BODY()

public:
	// 外部在创建单条提示控件后，通过这个接口填充名字/状态/时间
	void SetHintInfo(const FString& InPlayerName, const FString& InStateText, const FString& InTimeString);

private:
	//玩家名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
	//玩家状态
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerStateText;
	//提示时间
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TimeText;


};
