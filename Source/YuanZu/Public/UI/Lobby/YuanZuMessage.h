// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuMessage.generated.h"

class UTextBlock;

struct FYuanZuChatMessageInfo;

UCLASS()
class YUANZU_API UYuanZuMessage : public UUserWidget
{
	GENERATED_BODY()

private:
	//玩家名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
	//玩家发送的消息
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;

public:	
	void SetMessageInfo(const FYuanZuChatMessageInfo& InMessageInfo, int32 LocalPlayerId);
};
