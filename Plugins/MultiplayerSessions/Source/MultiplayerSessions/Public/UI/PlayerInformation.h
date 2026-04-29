// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Menu.h"
#include "MultiplayerInterface.h"
#include "PlayerInformation.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
class UWidgetAnimation;
class UWidgetSwitcher;
class UMenu;

UCLASS()
class MULTIPLAYERSESSIONS_API UPlayerInformation : public UUserWidget, public IMultiplayerInterface
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	//切换
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* UISwitchover;
	//玩家输入的名称
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PlayerInputName;
	//提示名字字符太多
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PromptText;
	//提示名字字符太多动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* PromptTextAnimation;
	//进入游戏
	UPROPERTY(meta = (BindWidget))
	UButton* EnterGameButton;
	//退出游戏
	UPROPERTY(meta = (BindWidget))
	UButton* QuitGameButton;
	//确认名字
	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmNameButton;

public:
	//取消名字
	UPROPERTY(meta = (BindWidget))
	UButton* CencelNameButton;
	//界面
	UPROPERTY(meta = (BindWidget))
	UMenu* WBP_Menu;

	//玩家名称
	FString PlayerName;

private:
	UFUNCTION()
	void OnEnterGame();
	UFUNCTION()
	void OnQuitGame();
	UFUNCTION()
	void OnPlayerInputName(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnPlayerInputChangedName(const FText& Text);
	UFUNCTION()
	void OnConfirmName();
	UFUNCTION()
	void OnCencelName();
	UFUNCTION()
	void OnClickedReturn();
	UFUNCTION()
	void OnHoveredReturn() { WBP_Menu->ButtonStyle(WBP_Menu->ReturnHeightLight, WBP_Menu->ReturnButton, WBP_Menu->ReturnText, EButtonState::EBS_Hovered); }
	UFUNCTION()
	void OnUnhoveredReturn() { WBP_Menu->ButtonStyle(WBP_Menu->ReturnHeightLight, WBP_Menu->ReturnButton, WBP_Menu->ReturnText, EButtonState::EBS_Unhovered); }
	UFUNCTION()
	void OnReleasedReturn() { WBP_Menu->ButtonStyle(WBP_Menu->ReturnHeightLight, WBP_Menu->ReturnButton, WBP_Menu->ReturnText, EButtonState::EBS_Released); }

	void PlayPromptTextAnimation();

	void UIOnly();
public:
	FORCEINLINE FString GetPlayerName()const { return PlayerName; }

	UFUNCTION(BlueprintCallable)
	UMenu* GetMenu() const {return WBP_Menu; }
};
