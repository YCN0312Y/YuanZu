#include "UI/Lobby/YuanZuMessage.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "UI/Lobby/Rests/YuanZuChatMessageInfo.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBoxSlot.h"

void UYuanZuMessage::SetMessageInfo(const FYuanZuChatMessageInfo& InMessageInfo, int32 LocalPlayerId)
{
	if (!PlayerNameText || !MessageText) return;

	PlayerNameText->SetText(FText::FromString(InMessageInfo.PlayerName));
	MessageText->SetText(FText::FromString(InMessageInfo.MessageText));

	// 1. 根据队伍设置名字颜色
	switch (InMessageInfo.TeamType)
	{
	case ETeamType::ETT_Red:
		PlayerNameText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.f)));
		break;

	case ETeamType::ETT_Blue:
		PlayerNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 1.f, 1.f)));
		break;

	case ETeamType::ETT_None:
	default:
		PlayerNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		break;
	}

	// 2. 判断是不是本机玩家发的
	const bool bIsLocalPlayerMessage = (InMessageInfo.SenderPlayerId == LocalPlayerId);

	// 3. 设置左右对齐
	if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(Slot))
	{
		ScrollSlot->SetHorizontalAlignment(bIsLocalPlayerMessage ? HAlign_Right : HAlign_Left);
	}

	PlayerNameText->SetJustification(bIsLocalPlayerMessage ? ETextJustify::Right : ETextJustify::Left);
	MessageText->SetJustification(bIsLocalPlayerMessage ? ETextJustify::Right : ETextJustify::Left);
}
