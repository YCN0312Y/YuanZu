#include "UI/Lobby/YuanZuPlayerStateHint.h"
#include "Components/TextBlock.h"

void UYuanZuPlayerStateHint::SetHintInfo(const FString& InPlayerName, const FString& InStateText, const FString& InTimeString)
{
	// 这里专门负责把数据层的文本写进单条提示控件
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InPlayerName));
	}

	if (PlayerStateText)
	{
		PlayerStateText->SetText(FText::FromString(InStateText));
	}

	if (TimeText)
	{
		TimeText->SetText(FText::FromString(InTimeString));
	}
}
