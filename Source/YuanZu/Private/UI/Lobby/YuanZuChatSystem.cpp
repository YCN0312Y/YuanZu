#include "UI/Lobby/YuanZuChatSystem.h"
#include "UI/Lobby/YuanZuMessage.h"
#include "UI/Lobby/YuanZuPlayerStateHint.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "Gameplay/Lobby/YuanZuLobbyGameState.h"
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/ScrollBox.h"
#include "TimerManager.h"
#include "UI/Lobby/YuanZuPlayerSettings.h"
#include "UI/Lobby/YuanZuTeamPlayer.h"

TWeakObjectPtr<UYuanZuPlayerSettings> UYuanZuTeamPlayer::CurrentOpenedPlayerSettings = nullptr;

bool UYuanZuChatSystem::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (SendButton)
	{
		SendButton->OnClicked.AddDynamic(this, &UYuanZuChatSystem::OnClickedSendButton);
	}

	return true;
}

void UYuanZuChatSystem::NativeConstruct()
{
	Super::NativeConstruct();
	TryBindLobbyGameState();
}

void UYuanZuChatSystem::NativeDestruct()
{
	if (CachedLobbyGameState)
	{
		CachedLobbyGameState->OnChatMessagesChanged.RemoveAll(this);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BindLobbyGameStateTimerHandle);
	}

	Super::NativeDestruct();
}

void UYuanZuChatSystem::TryBindLobbyGameState()
{
	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				BindLobbyGameStateTimerHandle,
				this,
				&UYuanZuChatSystem::TryBindLobbyGameState,
				0.1f,
				false
			);
		}
		return;
	}

	CachedLobbyGameState = LobbyGS;
	CachedLobbyGameState->OnChatMessagesChanged.RemoveAll(this);
	CachedLobbyGameState->OnChatMessagesChanged.AddUObject(this, &UYuanZuChatSystem::RefreshMessages);
	RefreshMessages(CachedLobbyGameState->GetChatMessages());
}

void UYuanZuChatSystem::OnClickedSendButton()
{
	if (!PlayerInputTextBox) return;

	const FString InputMessage = PlayerInputTextBox->GetText().ToString().TrimStartAndEnd();
	if (InputMessage.IsEmpty()) return;

	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!LPC) return;

	LPC->ServerSendChatMessage(InputMessage);
	PlayerInputTextBox->SetText(FText::GetEmpty());
}

void UYuanZuChatSystem::RefreshMessages(const TArray<FYuanZuChatMessageInfo>& InMessages)
{
	if (!MessageScrollBox) return;

	MessageScrollBox->ClearChildren();

	int32 LocalPlayerId = INDEX_NONE;
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (LPC && LPC->PlayerState)
	{
		LocalPlayerId = LPC->PlayerState->GetPlayerId();
	}

	for (const FYuanZuChatMessageInfo& MessageInfo : InMessages)
	{
		if (MessageInfo.bIsPlayerStateHint)
		{
			if (!PlayerStateHintClass) continue;

			//这类消息和普通聊天共用一个滚动框，但显示时要创建状态提示控件。
			UYuanZuPlayerStateHint* HintWidget = CreateWidget<UYuanZuPlayerStateHint>(GetOwningPlayer(), PlayerStateHintClass);
			if (!HintWidget) continue;

			MessageScrollBox->AddChild(HintWidget);
			HintWidget->SetHintInfo(MessageInfo.PlayerName, MessageInfo.MessageText, MessageInfo.TimeString);
			continue;
		}

		if (!MessageClass) continue;

		//普通聊天消息仍然沿用原来的聊天气泡控件。
		UYuanZuMessage* MessageWidget = CreateWidget<UYuanZuMessage>(GetOwningPlayer(), MessageClass);
		if (!MessageWidget) continue;

		MessageScrollBox->AddChild(MessageWidget);
		MessageWidget->SetMessageInfo(MessageInfo, LocalPlayerId);
	}

	MessageScrollBox->ScrollToEnd();
}