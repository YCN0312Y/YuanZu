#include "UI/Lobby/YuanZuTeamPlayer.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "Gameplay/Lobby/YuanZuLobbyGameMode.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerState.h"
#include "UI/Lobby/YuanZuPlayerSettings.h"
#include "GameFramework/PlayerState.h"
#include "Input/Reply.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Image.h"

void UYuanZuTeamPlayer::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (JoinTeamButton)
	{
		JoinTeamButton->OnClicked.AddDynamic(this, &UYuanZuTeamPlayer::OnClickedJoinTeamButton);
	}
}

void UYuanZuTeamPlayer::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (PrepareImage)
	{
		PrepareImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UYuanZuTeamPlayer::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UYuanZuTeamPlayer::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	const bool bIsHost = LPC && LPC->HasAuthority();

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bIsOccupied && bIsHost)
	{
		OpenRoomPlayerSettings(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UYuanZuTeamPlayer::OnClickedJoinTeamButton()
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!LPC) return;

	LPC->ServerRequestOccupyTeamSlot(TeamType, SlotIndex);
}

void UYuanZuTeamPlayer::OpenRoomPlayerSettings(const FVector2D& InScreenPosition)
{
	if (!RoomPlayerSettingsClass) return;

	if (CurrentOpenedPlayerSettings.IsValid())
	{
		CurrentOpenedPlayerSettings->RemoveFromParent();
		CurrentOpenedPlayerSettings = nullptr;
	}

	UYuanZuPlayerSettings* RoomPlayerSettings = CreateWidget<UYuanZuPlayerSettings>(GetOwningPlayer(), RoomPlayerSettingsClass);
	if (!RoomPlayerSettings) return;

	RoomPlayerSettings->AddToViewport(200);
	RoomPlayerSettings->InitRoomPlayerSettings(OccupyingPlayerName, OccupyingPlayerId);

	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::AbsoluteToViewport(this, InScreenPosition, PixelPosition, ViewportPosition);

	RoomPlayerSettings->SetMenuPosition(ViewportPosition);
	CurrentOpenedPlayerSettings = RoomPlayerSettings;
}

void UYuanZuTeamPlayer::InitSlotInfo(ETeamType InTeamType, int32 InSlotIndex)
{
	TeamType = InTeamType;
	SlotIndex = InSlotIndex;
}

void UYuanZuTeamPlayer::RefreshSlot(const FYuanZuTeamSlotInfo& SlotInfo)
{
	if (!JoinTeamButton || !TeamPlayerNameText || !TeamPlayerNameBorder || !PrepareImage) return;

	bIsOccupied = SlotInfo.bOccupied;
	OccupyingPlayerId = SlotInfo.OccupyingPlayer ? SlotInfo.OccupyingPlayer->GetPlayerId() : INDEX_NONE;

	//先判断这个插槽是否被占用
	if (SlotInfo.bOccupied)
	{
		//被占用
		JoinTeamButton->SetVisibility(ESlateVisibility::Hidden);
		TeamPlayerNameText->SetVisibility(ESlateVisibility::Visible);
		TeamPlayerNameBorder->SetVisibility(ESlateVisibility::Visible);

		OccupyingPlayerName = SlotInfo.PlayerName;
		TeamPlayerNameText->SetText(FText::FromString(OccupyingPlayerName));

		AYuanZuLobbyPlayerState* YuanZuLPS = Cast<AYuanZuLobbyPlayerState>(SlotInfo.OccupyingPlayer);
		if (YuanZuLPS)
		{
			const bool bShowPrepareImage = SlotInfo.bReady && !SlotInfo.bIsRoomOwner;
			PrepareImage->SetVisibility(bShowPrepareImage ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
		else
		{
			PrepareImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		//没被占用
		JoinTeamButton->SetVisibility(ESlateVisibility::Visible);
		TeamPlayerNameText->SetVisibility(ESlateVisibility::Hidden);
		TeamPlayerNameText->SetText(FText::GetEmpty());
		TeamPlayerNameBorder->SetVisibility(ESlateVisibility::Hidden);
		PrepareImage->SetVisibility(ESlateVisibility::Hidden);

		OccupyingPlayerName.Empty();
		OccupyingPlayerId = INDEX_NONE;
	}
}
