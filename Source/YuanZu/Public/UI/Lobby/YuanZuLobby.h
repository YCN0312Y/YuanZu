// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuLobby.generated.h"

class UYuanZuRoomSettings;
class UYuanZuChatSystem;
class UYuanZuPlayerJoinHint;
class UYuanZuPlayerList;
class UButton;
class UTexture2D;

UCLASS()
class YUANZU_API UYuanZuLobby : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized()override;
	virtual void NativePreConstruct()override;
	virtual void NativeConstruct()override;

private:
	//房间设置
	UPROPERTY(meta = (BindWidget))
	UYuanZuRoomSettings* RoomSettings;
	//隐藏房间侧边栏
	UPROPERTY(meta = (BindWidget))
	UButton* HiddenRoomSettingsButton;
	//聊天系统
	UPROPERTY(meta = (BindWidget))
	UYuanZuChatSystem* ChatSystem;
	//隐藏聊天侧边栏按钮
	UPROPERTY(meta = (BindWidget))
	UButton* HiddenChatSystemButton;
	//玩家状态提示
	UPROPERTY(meta = (BindWidget))
	UYuanZuPlayerJoinHint* PlayerJoinHint;
	//隐藏提示侧边栏按钮
	UPROPERTY(meta = (BindWidget))
	UButton* HiddenHintButton;
	//玩家列表
	UPROPERTY(meta = (BindWidget))
	UYuanZuPlayerList* PlayerList;
	//隐藏玩家列表侧边栏按钮
	UPROPERTY(meta = (BindWidget))
	UButton* HiddenPlayerListButton;
	//隐藏房间设置动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* RoomAnimation;
	//隐藏聊天系统动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ChatAnimation;
	//隐藏提示系统动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HintAnimation;
	//隐藏玩家列表动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* PlayerListAnimation;

	//隐层侧边栏后的显示图标样式
	UPROPERTY(EditAnywhere, Category = YuanZu)
	UTexture2D* L_HiddenTexture;
	UPROPERTY(EditAnywhere, Category = YuanZu)
	UTexture2D* L_VisibleTexture;	
	UPROPERTY(EditAnywhere, Category = YuanZu)
	UTexture2D* R_HiddenTexture;
	UPROPERTY(EditAnywhere, Category = YuanZu)
	UTexture2D* R_VisibleTexture;

	//检查是否已经隐藏房间设置
	bool bIsHiddenRoomSettings = false;
	//检查是否已经隐藏聊天系统
	bool bIsHiddenChatSystem = false;
	//检查是否已经隐藏玩家加入提示
	bool bIsHiddenHint = false;
	//检查是否已经隐藏玩家列表
	bool bIsHiddenPlayerList = false;

private:
	UFUNCTION()
	void OnClickedHiddenRoomSettingsButton();
	UFUNCTION()
	void OnClickedHiddenChatSystemButton();
	UFUNCTION()
	void OnClickedHiddenHintButton();
	UFUNCTION()
	void OnClickedHiddenPlayerListButton();

public:
	void SetButtonStyle(UButton* NewButton, UTexture2D* NewTexture);

};
