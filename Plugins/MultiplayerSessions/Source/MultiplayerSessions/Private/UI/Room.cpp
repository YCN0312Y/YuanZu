// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Room.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "MultiplayerSessions/Log/YUYI_MlutiplayerLog.h"
#include "UI/Menu.h"

void URoom::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (GameRoomButton)
	{
		GameRoomButton->OnClicked.AddDynamic(this, &URoom::OnGameRoomClicked);
	}
}

void URoom::NativeDestruct()
{
	Super::NativeDestruct();
}

void URoom::Init(const FRoomInformation& Info, int32 NewIndex)
{
	Index = NewIndex;

	UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("URoom::Init Info.RoomName = [%s]"), *Info.RoomName);

	RoomNameText->SetText(FText::FromString(Info.RoomName));
	CurrentPlayerNumberText->SetText(FText::AsNumber(Info.RoomCurrentPlayerNumber));
	MaxPlayerNumberText->SetText(FText::AsNumber(Info.RoomMaxPlayerNumber));
	PingText->SetText(FText::AsNumber(Info.RoomPing));
	SetPingImage(Info.RoomPing);

}

void URoom::SetSelectedState(bool bIsSelected)
{
	if (GameRoomButton)
	{
		//如果被选中了，禁用按钮(不可再次点击)如果未选中，启用按钮(可点击)
		GameRoomButton->SetIsEnabled(!bIsSelected);
	}
}

void URoom::OnGameRoomClicked()
{
	OnRoomClicked.Broadcast(this);
}

void URoom::SetPingImage(int32 PingValue)
{
	if (MSImage && !MS.IsEmpty())
	{
		UTexture2D* Texture = nullptr;

		if (PingValue > 0)
		{
			if (PingValue < 101)
			{
				Texture = MS[0];
			}
			else if (PingValue < 201)
			{
				Texture = MS[1];
			}
			else if (PingValue < 301)
			{
				Texture = MS[2];
			}
			else
			{
				Texture = MS[2];
			}
			if (Texture)
			{
				MSImage->SetBrushFromTexture(Texture);
			}
		}
	}
}
