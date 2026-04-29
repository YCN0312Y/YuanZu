// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Lobby/YuanZuLobbyHUD.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "UI/Lobby/YuanZuLobby.h"

void AYuanZuLobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	CreateLobbyWidget();
}

void AYuanZuLobbyHUD::CreateLobbyWidget()
{
	AYuanZuLobbyPlayerController* YuanZuLPC = Cast<AYuanZuLobbyPlayerController>(GetOwningPlayerController());
	if (YuanZuLPC)
	{
		if (LobbyClass)
		{
			Lobby = CreateWidget<UYuanZuLobby>(YuanZuLPC, LobbyClass);
			if (Lobby)
			{
				Lobby->SetIsFocusable(true);//聚焦
				if (GetWorld())
				{
					FInputModeUIOnly InputModeData;//输入模式仅用户界面
					InputModeData.SetWidgetToFocus(Lobby->TakeWidget());//将小部件设为焦点状态
					InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//将鼠标锁定设置为视口模式行为模式为：不将鼠标锁定在视图区域内			
					YuanZuLPC->SetInputMode(InputModeData);//设置输入模式
					YuanZuLPC->SetShowMouseCursor(true);//设置显示鼠标
				}
				Lobby->AddToViewport();
			}
		}
	}
}
