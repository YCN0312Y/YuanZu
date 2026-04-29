// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/PlayerInformation.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"
#include "Containers/EnumAsByte.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Menu.h"
#include "Components/Border.h"
#include "MultiplayerSessionsSubsystem.h"

void UPlayerInformation::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PromptText)
	{
		PromptText->SetVisibility(ESlateVisibility::Hidden);
	}
	if (PlayerInputName)
	{
		PlayerInputName->OnTextChanged.AddDynamic(this, &UPlayerInformation::OnPlayerInputChangedName);
		PlayerInputName->OnTextCommitted.AddDynamic(this, &UPlayerInformation::OnPlayerInputName);
	}
	if (EnterGameButton)
	{
		EnterGameButton->OnPressed.AddDynamic(this, &UPlayerInformation::OnEnterGame);
	}
	if (QuitGameButton)
	{
		QuitGameButton->OnPressed.AddDynamic(this, &UPlayerInformation::OnQuitGame);
	}
	if (ConfirmNameButton)
	{
		ConfirmNameButton->OnPressed.AddDynamic(this, &UPlayerInformation::OnConfirmName);
	}
	if (CencelNameButton)
	{
		CencelNameButton->OnPressed.AddDynamic(this, &UPlayerInformation::OnCencelName);
	}
	if (WBP_Menu->ReturnButton)
	{

		WBP_Menu->ReturnButton->OnHovered.AddDynamic(this, &UPlayerInformation::OnHoveredReturn);
		WBP_Menu->ReturnButton->OnUnhovered.AddDynamic(this, &UPlayerInformation::OnUnhoveredReturn);
		WBP_Menu->ReturnButton->OnClicked.AddDynamic(this, &UPlayerInformation::OnClickedReturn);
		WBP_Menu->ReturnButton->OnReleased.AddDynamic(this, &UPlayerInformation::OnReleasedReturn);

	}
}

void UPlayerInformation::NativeConstruct()
{
	Super::NativeConstruct();
	UIOnly();
}

void UPlayerInformation::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPlayerInformation::OnEnterGame()
{
	if (UISwitchover)
	{
		UISwitchover->SetActiveWidgetIndex(1);
	}

}

void UPlayerInformation::OnQuitGame()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;
	UKismetSystemLibrary::QuitGame(this, PlayerController, QuitPreference, false);
}

//文本更新就触发
void UPlayerInformation::OnPlayerInputName(const FText& Text, ETextCommit::Type CommitMethod)
{
	PlayerName = Text.ToString();

	if (PlayerName.Len() < 7 && PlayerName.Len() > 0)
	{
		if (PlayerName.Contains(TEXT(" ")))
		{
			PromptText->SetVisibility(ESlateVisibility::Visible);
			PromptText->SetText(FText::FromString(TEXT("名称中不得有空白符号！！！")));

		}
		PlayerName = PlayerName.Left(7);
		PlayerInputName->SetText(FText::FromString(PlayerName));

		UE_LOG(LogTemp, Warning, TEXT("ChengGong[%s]"), *PlayerName);
	}
	else if (PromptTextAnimation && PlayerName.Len() > 6)
	{
		PromptText->SetVisibility(ESlateVisibility::Visible);
		PromptText->SetText(FText::FromString(TEXT("输入错误，名称不得超过6个字符！")));
		PlayPromptTextAnimation();
	}
}

//文本提交就触发
void UPlayerInformation::OnPlayerInputChangedName(const FText& Text)
{
	PlayerName = Text.ToString();

	FEditableTextBoxStyle NewStyle = PlayerInputName->WidgetStyle;
	NewStyle.TextStyle.SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f));
	PlayerInputName->WidgetStyle = NewStyle;
	PlayerInputName->SynchronizeProperties();

}

void UPlayerInformation::OnConfirmName()
{
	if (PlayerName.Len() == 0)
	{
		PromptText->SetVisibility(ESlateVisibility::Visible);
		PromptText->SetText(FText::FromString(TEXT("输入错误，名称不得为空！")));
		PlayPromptTextAnimation();
		return;
	}

	if (PlayerName.Len() > 0 && PlayerName.Len() < 7)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMultiplayerSessionsSubsystem* MPSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
			{
				MPSubsystem->SetCachedPlayerName(PlayerName);
				UE_LOG(LogTemp, Warning, TEXT("Cached PlayerName = %s"), *PlayerName);
			}
		}
	}

	if (UISwitchover && PlayerName.Len() > 0 && PlayerName.Len() < 7 && WBP_Menu->GetCreateButton())
	{
		WBP_Menu->OnClickedCreate();
		UISwitchover->SetActiveWidgetIndex(2);
	}
}

void UPlayerInformation::OnCencelName()
{
	if (UISwitchover)
	{
		UISwitchover->SetActiveWidgetIndex(0);
	}
}

void UPlayerInformation::OnClickedReturn()
{
	if (WBP_Menu)
	{
		WBP_Menu->OnReleasedJoin();
		WBP_Menu->ButtonStyle(WBP_Menu->ReturnHeightLight, WBP_Menu->ReturnButton, WBP_Menu->ReturnText, EButtonState::EBS_Pressed);
	}
	UISwitchover->SetActiveWidgetIndex(1);
}

void UPlayerInformation::PlayPromptTextAnimation()
{
	PlayAnimation(PromptTextAnimation);
	FEditableTextBoxStyle Style = PlayerInputName->WidgetStyle;
	Style.TextStyle.SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f));
	PlayerInputName->WidgetStyle = Style;
	PlayerInputName->SynchronizeProperties();
}

void UPlayerInformation::UIOnly()
{
	SetIsFocusable(true);//聚焦
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();//获取首个玩家控制器
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;//输入模式仅用户界面
			InputModeData.SetWidgetToFocus(TakeWidget());//将小部件设为焦点状态
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//将鼠标锁定设置为视口模式行为模式为：不将鼠标锁定在视图区域内			
			PlayerController->SetInputMode(InputModeData);//设置输入模式
			PlayerController->SetShowMouseCursor(true);//设置显示鼠标
		}
	}
}
