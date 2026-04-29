// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuGameSettings.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class YUANZU_API UYuanZuGameSettings : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

private:
	//关闭设置
	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;
	//按键
	UPROPERTY(meta = (BindWidget))
	UButton* KeyButton;
	//按键文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KeyText;
	//图像
	UPROPERTY(meta = (BindWidget))
	UButton* ImageButton;
	//图像文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ImageText;
	//声音
	UPROPERTY(meta = (BindWidget))
	UButton* SoundButton;
	//声音文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SoundText;
	//消息
	UPROPERTY(meta = (BindWidget))
	UButton* MessageButton;
	//消息文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;
	//语言
	UPROPERTY(meta = (BindWidget))
	UButton* LanguageButton;
	//语言文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LanguageText;
	//账户
	UPROPERTY(meta = (BindWidget))
	UButton* AccountButton;
	//账户文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AccountText;
	//其他
	UPROPERTY(meta = (BindWidget))
	UButton* MsicButton;
	//其他文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MsicText;
	//退出游戏
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	//退出游戏文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuitText;
	//确认是否退出控件
	UPROPERTY(meta = (BindWidget))
	UUserWidget* CancelQuit;



public:
	//禁用了的
	UPROPERTY(EditAnywhere, Category = YuanZu)
	float MaxFontInfoSize;
	//控制
	UPROPERTY(meta = (BindWidget))
	UButton* ControllerButton;
	//控制文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ControllerText;

	UTextBlock* LastText = nullptr;
	UButton* LastButton = nullptr;

private:
	UFUNCTION()
	void OnClickedCancelButton();
	UFUNCTION()
	void OnClickedControllerButton();
	UFUNCTION()
	void OnClickedKeyButton();
	UFUNCTION()
	void OnClickedImageButton();
	UFUNCTION()
	void OnClickedSoundButton();
	UFUNCTION()
	void OnClickedMessageButton();
	UFUNCTION()
	void OnClickedLanguageButton();
	UFUNCTION()
	void OnClickedAccountButton();
	UFUNCTION()
	void OnClickedMsicButton();
	UFUNCTION()
	void OnClickedQuitButton();

	void SetTextStyle(UTextBlock* Text, UButton* Button);

public:
	void DefaultSetting();
};
