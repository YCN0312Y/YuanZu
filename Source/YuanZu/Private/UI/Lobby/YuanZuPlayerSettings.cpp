#include "UI/Lobby/YuanZuPlayerSettings.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "Gameplay/Lobby/YuanZuLobbyGameMode.h"
#include "Blueprint/WidgetLayoutLibrary.h"

bool UYuanZuPlayerSettings::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (RemovePlayerButton)
	{
		RemovePlayerButton->OnClicked.AddDynamic(this, &UYuanZuPlayerSettings::OnClickedRemoveButton);
	}

	if (BannedPostButton)
	{
		BannedPostButton->OnClicked.AddDynamic(this, &UYuanZuPlayerSettings::OnClickedBannedPostButton);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UYuanZuPlayerSettings::OnClickedCloseButton);
	}

	return true;
}

void UYuanZuPlayerSettings::NativeConstruct()
{
	Super::NativeConstruct();

	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	const bool bIsHost = LPC && LPC->HasAuthority();

	if (RemovePlayerButton)
	{
		RemovePlayerButton->SetIsEnabled(bIsHost);
		RemovePlayerButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (BannedPostButton)
	{
		BannedPostButton->SetIsEnabled(bIsHost);
		BannedPostButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (CloseButton)
	{
		CloseButton->SetIsEnabled(true);
	}
}

void UYuanZuPlayerSettings::InitRoomPlayerSettings(const FString& InPlayerName, int32 InTargetPlayerId)
{
	TargetPlayerName = InPlayerName;
	TargetPlayerId = InTargetPlayerId;

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(TargetPlayerName));
	}

	RefreshBannedPostText();
}

void UYuanZuPlayerSettings::SetMenuPosition(const FVector2D& InViewportPosition)
{
	FVector2D FinalPos = InViewportPosition;

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);

	// 这里的 220、120 你按自己菜单实际尺寸改
	FinalPos.X = FMath::Clamp(FinalPos.X, 0.f, ViewportSize.X - 220.f);
	FinalPos.Y = FMath::Clamp(FinalPos.Y, 0.f, ViewportSize.Y - 120.f);

	SetPositionInViewport(FinalPos, false);
}

void UYuanZuPlayerSettings::RefreshBannedPostText()
{
	AYuanZuLobbyGameMode* LobbyGM = GetWorld() ? GetWorld()->GetAuthGameMode<AYuanZuLobbyGameMode>() : nullptr;
	if (LobbyGM && TargetPlayerId != INDEX_NONE)
	{
		bIsTargetMuted = LobbyGM->IsPlayerMuted(TargetPlayerId);
	}

	if (BannedPostText)
	{
		BannedPostText->SetText(FText::FromString(bIsTargetMuted ? RemoveBannedPost : BannedPost));
	}
}

void UYuanZuPlayerSettings::OnClickedRemoveButton()
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!LPC || !LPC->HasAuthority()) return;

	LPC->ServerRequestKickPlayer(TargetPlayerId);
	RemoveFromParent();
}

void UYuanZuPlayerSettings::OnClickedBannedPostButton()
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!LPC) return;

	//这里先调用服务器RPC
	LPC->ServerRequestToggleMutePlayer(TargetPlayerId);

	//刷新禁言文本
	RefreshBannedPostText();
	//从父项中移除
	RemoveFromParent();
}

void UYuanZuPlayerSettings::OnClickedCloseButton()
{
	RemoveFromParent();
}