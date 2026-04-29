#include "UI/Lobby/YuanZuJoinPlayerMessage.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerState.h"

void UYuanZuJoinPlayerMessage::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (RemoveButton)
	{
		UpdateRemoveButton();
		RemoveButton->OnClicked.AddDynamic(this, &UYuanZuJoinPlayerMessage::OnClickedRemoveButton);
	}
}

void UYuanZuJoinPlayerMessage::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UYuanZuJoinPlayerMessage::NativeConstruct()
{
	Super::NativeConstruct();
}

void UYuanZuJoinPlayerMessage::OnClickedRemoveButton()
{
	AYuanZuLobbyPlayerController* YuanZuLPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!YuanZuLPC) return;

	YuanZuLPC->ServerRequestKickPlayer(TargetPlayerID);
}

void UYuanZuJoinPlayerMessage::InitJoinPlayerMessage(const FYuanZuPlayerData& InPlayerData)
{
	if (!PlayerNameText || !RemoveButton) return;

	TargetPlayerName = InPlayerData.PlayerName;
	TargetPlayerID = InPlayerData.PlayerId;
	PlayerNameText->SetText(FText::FromString(TargetPlayerName));

	UpdateRemoveButton();
}

void UYuanZuJoinPlayerMessage::UpdateRemoveButton()
{
	AYuanZuLobbyPlayerController* YuanZuLPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!YuanZuLPC || !RemoveButton) return;

	AYuanZuLobbyPlayerState* YuanZuLPS = YuanZuLPC->GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS) return;

	RemoveButton->SetVisibility(YuanZuLPS->bIsRoomOwner ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
