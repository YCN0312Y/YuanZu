// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuAnnouncement.generated.h"

class UTextBlock;

UCLASS()
class YUANZU_API UYuanZuAnnouncement : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//公告文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;
	//等待比赛开始倒计时
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarupTimeText;
};
