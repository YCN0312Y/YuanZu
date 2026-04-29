
#include "UI/Lobby/YuanZuLobby.h"
#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "UI/Lobby/YuanZuPlayerJoinHint.h"

void UYuanZuLobby::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (HiddenRoomSettingsButton)
	{
		HiddenRoomSettingsButton->OnClicked.AddDynamic(this, &UYuanZuLobby::OnClickedHiddenRoomSettingsButton);
	}
	if (HiddenChatSystemButton)
	{
		HiddenChatSystemButton->OnClicked.AddDynamic(this, &UYuanZuLobby::OnClickedHiddenChatSystemButton);
	}
	if (HiddenHintButton)
	{
		HiddenHintButton->OnClicked.AddDynamic(this, &UYuanZuLobby::OnClickedHiddenHintButton);
	}
	if (HiddenPlayerListButton)
	{
		HiddenPlayerListButton->OnClicked.AddDynamic(this, &UYuanZuLobby::OnClickedHiddenPlayerListButton);
	}
}

void UYuanZuLobby::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UYuanZuLobby::NativeConstruct()
{
	Super::NativeConstruct();
}

void UYuanZuLobby::OnClickedHiddenRoomSettingsButton()
{
	if (!bIsHiddenRoomSettings)
	{
		bIsHiddenRoomSettings = true;
		if (RoomAnimation)
		{
			PlayAnimationForward(RoomAnimation);
			SetButtonStyle(HiddenRoomSettingsButton, L_VisibleTexture);
		}
	}
	else
	{
		bIsHiddenRoomSettings = false;
		if (RoomAnimation)
		{
			PlayAnimationReverse(RoomAnimation);
			SetButtonStyle(HiddenRoomSettingsButton, L_HiddenTexture);
		}
	}

}

void UYuanZuLobby::OnClickedHiddenChatSystemButton()
{
	if (!bIsHiddenChatSystem)
	{
		bIsHiddenChatSystem = true;
		if (ChatAnimation)
		{
			PlayAnimationForward(ChatAnimation);
			SetButtonStyle(HiddenChatSystemButton, L_VisibleTexture);
		}
	}
	else
	{
		bIsHiddenChatSystem = false;
		if (ChatAnimation)
		{
			PlayAnimationReverse(ChatAnimation);
			SetButtonStyle(HiddenChatSystemButton, L_HiddenTexture);
		}
	}
}

void UYuanZuLobby::OnClickedHiddenHintButton()
{
	if (!bIsHiddenHint)
	{
		bIsHiddenHint = true;
		if (HintAnimation)
		{
			PlayAnimationForward(HintAnimation);
			SetButtonStyle(HiddenHintButton, R_VisibleTexture);
		}
	}
	else
	{
		bIsHiddenHint = false;
		if (HintAnimation)
		{
			PlayAnimationReverse(HintAnimation);
			SetButtonStyle(HiddenHintButton, R_HiddenTexture);
		}
	}
}

void UYuanZuLobby::OnClickedHiddenPlayerListButton()
{
	if (!bIsHiddenPlayerList)
	{
		bIsHiddenPlayerList = true;
		if (PlayerListAnimation)
		{
			PlayAnimationForward(PlayerListAnimation);
			SetButtonStyle(HiddenPlayerListButton, R_VisibleTexture);
		}
	}
	else
	{
		bIsHiddenPlayerList = false;
		if (PlayerListAnimation)
		{
			PlayAnimationReverse(PlayerListAnimation);
			SetButtonStyle(HiddenPlayerListButton, R_HiddenTexture);
		}
	}
}

void UYuanZuLobby::SetButtonStyle(UButton* NewButton, UTexture2D* NewTexture)
{
	FButtonStyle NewStyle = NewButton->GetStyle();
	FSlateBrush NormalBrush;
	NormalBrush.SetResourceObject(NewTexture);
	NormalBrush.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.8f));
	NormalBrush.ImageSize = FVector2D(32.f, 32.f);
	FSlateBrush HoveredBrush;
	HoveredBrush.SetResourceObject(NewTexture);
	HoveredBrush.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.5f));
	HoveredBrush.ImageSize = FVector2D(34.5f, 34.5f);
	NewStyle.SetNormal(NormalBrush);
	NewStyle.SetHovered(HoveredBrush);
	NewStyle.SetPressed(HoveredBrush);
	NewButton->SetStyle(NewStyle);
}
