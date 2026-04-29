// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/YuanZuGameSettings.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Gameplay/YuanZuPlayerController.h"

bool UYuanZuGameSettings::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedCancelButton);
	}
	if (ControllerButton)
	{
		ControllerButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedControllerButton);
	}
	if (KeyButton)
	{
		KeyButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedKeyButton);
	}
	if (ImageButton)
	{
		ImageButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedImageButton);
	}
	if (SoundButton)
	{
		SoundButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedSoundButton);
	}
	if (MessageButton)
	{
		MessageButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedMessageButton);
	}
	if (LanguageButton)
	{
		LanguageButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedLanguageButton);
	}
	if (AccountButton)
	{
		AccountButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedAccountButton);
	}
	if (MsicButton)
	{
		MsicButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedMsicButton);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UYuanZuGameSettings::OnClickedQuitButton);
	}

	DefaultSetting();

	return true;
}

void UYuanZuGameSettings::OnClickedCancelButton()
{
	if (AYuanZuPlayerController* YuanZuPC = Cast<AYuanZuPlayerController>(GetOwningPlayer()))
	{
		YuanZuPC->CloseSetting();
	}
}

void UYuanZuGameSettings::OnClickedControllerButton()
{
	if (LastButton != ControllerButton && LastText != ControllerText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = ControllerButton;
		LastText = ControllerText;
		SetTextStyle(ControllerText, ControllerButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedKeyButton()
{
	if (LastButton != KeyButton && LastText != KeyText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = KeyButton;
		LastText = KeyText;
		SetTextStyle(KeyText, KeyButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedImageButton()
{
	if (LastButton != ImageButton && LastText != ImageText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = ImageButton;
		LastText = ImageText;
		SetTextStyle(ImageText, ImageButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedSoundButton()
{
	if (LastButton != SoundButton && LastText != SoundText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = SoundButton;
		LastText = SoundText;
		SetTextStyle(SoundText, SoundButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedMessageButton()
{
	if (LastButton != MessageButton && LastText != MessageText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = MessageButton;
		LastText = MessageText;
		SetTextStyle(MessageText, MessageButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedLanguageButton()
{
	if (LastButton != LanguageButton && LastText != LanguageText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = LanguageButton;
		LastText = LanguageText;
		SetTextStyle(LanguageText, LanguageButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedAccountButton()
{
	if (LastButton != AccountButton && LastText != AccountText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = AccountButton;
		LastText = AccountText;
		SetTextStyle(AccountText, AccountButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedMsicButton()
{
	if (LastButton != MsicButton && LastText != MsicText)
	{
		SetTextStyle(LastText, LastButton);
		LastButton = MsicButton;
		LastText = MsicText;
		SetTextStyle(MsicText, MsicButton);
	}
	else
	{
		return;
	}
}

void UYuanZuGameSettings::OnClickedQuitButton()
{
	CancelQuit->SetVisibility(ESlateVisibility::Visible);
}

void UYuanZuGameSettings::SetTextStyle(UTextBlock* Text, UButton* Button)
{
	if (!Text && !Button && !LastButton) return;

	if (LastButton->GetIsEnabled())
	{
		Button->SetIsEnabled(false);
		FSlateFontInfo MaxFontInfo = Text->GetFont();
		MaxFontInfo.Size = MaxFontInfoSize;
		Text->SetFont(MaxFontInfo);
	}
	else
	{
		Button->SetIsEnabled(true);
		FSlateFontInfo MinFontInfo = Text->GetFont();
		MinFontInfo.Size = 32;
		Text->SetFont(MinFontInfo);
	}
}

void UYuanZuGameSettings::DefaultSetting()
{
	// 默认打开设置时，控制页签就是选中状态
	LastButton = ControllerButton;
	LastText = ControllerText;

	if (ControllerButton)
	{
		ControllerButton->SetIsEnabled(false);
	}

	if (ControllerText)
	{
		FSlateFontInfo FontInfo = ControllerText->GetFont();
		FontInfo.Size = MaxFontInfoSize;
		ControllerText->SetFont(FontInfo);
	}
}
