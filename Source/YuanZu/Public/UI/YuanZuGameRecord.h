// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuGameRecord.generated.h"

class UVerticalBox;
class UYuanZuPlayerRecord;

UCLASS()
class YUANZU_API UYuanZuGameRecord : public UUserWidget
{
	GENERATED_BODY()
	
private:
	//红队
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RedTeam;
	//蓝队
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* BlueTeam;
	//玩家战绩
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuPlayerRecord>PlayerRecord;

public:
	void RefreshRecordList();

};
