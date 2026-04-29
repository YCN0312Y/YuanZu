// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuTeamPlayer.generated.h"

class UButton;
class UTextBlock;
class UBorder;
class UYuanZuPlayerSettings;
class UImage;

UCLASS()
class YUANZU_API UYuanZuTeamPlayer : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct()override;
	virtual void NativeConstruct() override;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	//加入队伍
	UPROPERTY(meta = (BindWidget))
	UButton* JoinTeamButton;
	//加入队伍的玩家名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TeamPlayerNameText;
	//玩家名称背景
	UPROPERTY(meta = (BindWidget))
	UBorder* TeamPlayerNameBorder;
	//准备图标
	UPROPERTY(meta = (BindWidget))
	UImage* PrepareImage;

	//队伍阵容
	UPROPERTY()
	ETeamType TeamType = ETeamType::ETT_None;
	//插槽索引
	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	//当前槽位是否被占用
	UPROPERTY()
	bool bIsOccupied = false;

	//当前槽位玩家名
	UPROPERTY()
	FString OccupyingPlayerName;

	//当前槽位玩家ID
	UPROPERTY()
	int32 OccupyingPlayerId = INDEX_NONE;

	//右键菜单类
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuPlayerSettings> RoomPlayerSettingsClass;

	//当前打开的右键菜单（全局只开一个）
	static TWeakObjectPtr<UYuanZuPlayerSettings> CurrentOpenedPlayerSettings;

private:
	//加入队伍
	UFUNCTION()
	void OnClickedJoinTeamButton();

	void OpenRoomPlayerSettings(const FVector2D& InScreenPosition);

public:
	//初始化槽位信息
	void InitSlotInfo(ETeamType InTeamType, int32 InSlotIndex);
	//刷新插槽
	void RefreshSlot(const FYuanZuTeamSlotInfo& SlotInfo);
};
