#include "UI/Lobby/YuanZuPlayerList.h"

#include "Components/ScrollBox.h"
#include "Gameplay/Lobby/YuanZuLobbyGameState.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "UI/Lobby/YuanZuJoinPlayerMessage.h"
#include "UI/Lobby/Rests/YuanZuPlayerData.h"

void UYuanZuPlayerList::NativeConstruct()
{
	Super::NativeConstruct();

	AYuanZuLobbyGameState* YuanZuLGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!YuanZuLGS) return;

	YuanZuLGS->OnPlayerListChanged.RemoveAll(this);
	YuanZuLGS->OnPlayerListChanged.AddUObject(this, &UYuanZuPlayerList::RefreshPlayerList);

	RefreshPlayerList();
}

void UYuanZuPlayerList::NativeDestruct()
{
	AYuanZuLobbyGameState* YuanZuLGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (YuanZuLGS)
	{
		YuanZuLGS->OnPlayerListChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UYuanZuPlayerList::RefreshPlayerList()
{
	if (!PlayerListScrollBox || !JoinPlayerMessageClass) return;

	PlayerListScrollBox->ClearChildren();

	AYuanZuLobbyGameState* YuanZuLGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!YuanZuLGS) return;

	AYuanZuLobbyPlayerController* YuanZuLPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!YuanZuLPC) return;

	for (const FYuanZuPlayerData& LobbyPlayer : YuanZuLGS->GetLobbyPlayers())
	{
		if (LobbyPlayer.bInTeam)
		{
			continue;
		}

		JoinPlayerMessage = CreateWidget<UYuanZuJoinPlayerMessage>(YuanZuLPC, JoinPlayerMessageClass);
		if (JoinPlayerMessage)
		{
			JoinPlayerMessage->InitJoinPlayerMessage(LobbyPlayer);
			PlayerListScrollBox->AddChild(JoinPlayerMessage);
		}
	}
}
