// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuRoomSettings.generated.h"

class UComboBoxString;
class UButton;
class UTextBlock;
class UBorder;

struct FYuanZuMapOptionData;
struct FYuanZuTeamSlotInfo;

UCLASS()
class YUANZU_API UYuanZuRoomSettings : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

private:
	//地图选择
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* MapComboBox;
	//开始游戏文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StartGameText;
	//开始游戏按钮
	UPROPERTY(meta = (BindWidget))
	UButton* StartGameButton;
	//退出队伍文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuitTeamText;
	//退出队伍按钮
	UPROPERTY(meta = (BindWidget))
	UButton* QuitTeamButton;
	//客户端看到房主选的的地图的背景
	UPROPERTY(meta = (BindWidget))
	UBorder* MapNameTextBorder;
	//客户端看到房主选的的地图的文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapNameText;
	//地图数据
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TArray<FYuanZuMapOptionData> MapData;
	//地图路径
	FString MapPath;

private:
	//选择地图
	UFUNCTION()
	void OnSelectionChangedMap(FString SelectedItem, ESelectInfo::Type SelectionType);
	//开始按钮
	UFUNCTION()
	void OnClickedStartGameButton();
	//退出队伍
	UFUNCTION()
	void OnClickedQuitTeamButton();

	//刷新选择地图的文本
	void RefreshSelectedMapText(const FString& InMapDisplayName);
	//队伍槽位变化后，用它中转刷新开始按钮文本和可点击状态。
	void HandleTeamSlotsChanged(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots);
	//刷新开始游戏按钮状态
	void RefreshStartGameButtonState();
	//更新退出队伍按钮状态
	void UpdateQuitTeamButtonState(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots);
	//根据房主和房员设置UI
	void UpdateRoomSettingsAuthorityUI();
	//更新地图数量
	void UpdateMapCount();
	//更新地图路径
	bool UpdateMapPathByDisplayName(const FString& InSelectedMapName);
	// 检查本地玩家是否已经进入队伍，并返回当前准备状态。
	bool IsLocalPlayerInTeam(bool& bOutReady) const;
	// 房主用它判断当前是否允许开始游戏。
	bool AreAllOccupiedPlayersReady() const;
};
