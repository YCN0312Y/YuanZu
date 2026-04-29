// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/Menu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Border.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Animation/WidgetAnimation.h"
#include "Components/CircularThrobber.h"
#include "Components/SizeBox.h"
#include "TimerManager.h"
#include "UI/Room.h"
#include "Components/ScrollBox.h"
#include "Components/Overlay.h"

void UMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	LastSelectedRoom = nullptr;

	if (CreateGameButton)
	{
		CreateGameButton->OnClicked.AddDynamic(this, &UMenu::CreateButtonClicked);
	}
	if (JoinGameButton)
	{
		JoinGameButton->SetIsEnabled(false);
		JoinGameButton->OnHovered.AddDynamic(this, &UMenu::OnHoveredJoinGame);
		JoinGameButton->OnUnhovered.AddDynamic(this, &UMenu::OnUnhoveredJoinGame);
		JoinGameButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
		JoinGameButton->OnReleased.AddDynamic(this, &UMenu::OnReleasedJoinGame);
	}
	if (CreateButton)
	{
		CreateButton->OnHovered.AddDynamic(this, &UMenu::OnHoveredCreate);
		CreateButton->OnUnhovered.AddDynamic(this, &UMenu::OnUnhoveredCreate);
		CreateButton->OnClicked.AddDynamic(this, &UMenu::OnClickedCreate);
	}
	if (JoinButton)
	{
		JoinButton->OnHovered.AddDynamic(this, &UMenu::OnHoveredJoin);
		JoinButton->OnUnhovered.AddDynamic(this, &UMenu::OnUnhoveredJoin);
		JoinButton->OnClicked.AddDynamic(this, &UMenu::OnClickedJoin);
	}
	if (CreateHeightLight)
	{
		CreateHeightLight->SetVisibility(ESlateVisibility::Hidden);
	}
	if (JoinHeightLight)
	{
		JoinHeightLight->SetVisibility(ESlateVisibility::Hidden);
	}
	if (ReturnHeightLight)
	{
		ReturnHeightLight->SetVisibility(ESlateVisibility::Hidden);
	}
	if (AddPeople)
	{
		AddPeople->OnClicked.AddDynamic(this, &UMenu::OnAddPeople);
	}
	if (SubPeople)
	{
		SubPeople->OnClicked.AddDynamic(this, &UMenu::OnSubPeople);
	}
	if (InputPeopleNumber)
	{
		InputPeopleNumber->OnTextChanged.AddDynamic(this, &UMenu::OnInputPeopleNumberChanged);
		InputPeopleNumber->OnTextCommitted.AddDynamic(this, &UMenu::OnInputPeopleNumberCommitted);
	}
	if (PlayerNumberText)
	{
		PlayerNumberText->SetVisibility(ESlateVisibility::Hidden);
	}
	if (SearchGameButton)
	{
		SearchGameButton->OnClicked.AddDynamic(this, &UMenu::OnClickedSearchGame);
	}
	if (SearchCircularThrobber)
	{
		SearchCircularThrobber->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->OnFindRoomsComplete.AddUObject(this, &UMenu::OnFindRoom);
	}
	if (CreateRoomNameText)
	{
		CreateRoomNameText->OnTextChanged.AddDynamic(this, &UMenu::OnCreateRoomNameTextChanged);
		CreateRoomNameText->OnTextCommitted.AddDynamic(this, &UMenu::OnCreateRoomNameTextCommitted);
	}
	if (RoomHintText)
	{
		RoomHintText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.RemoveAll(this);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.RemoveAll(this);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.RemoveAll(this);

		MultiplayerSessionsSubsystem->OnFindRoomsComplete.RemoveAll(this);
	}
}

//菜单设置
void UMenu::MenuSetup(FString TypeToMatch, FString HallPath)
{
	PathToHall = FString::Printf(TEXT("%s?listen"), *HallPath);
	//NumPublicConnections = NumberPublicConnections;
	MatchType = TypeToMatch;

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMenu::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);

		MultiplayerSessionsSubsystem->OnFindRoomsComplete.AddUObject(this, &UMenu::OnFindRoom);
	}
}

void UMenu::ButtonSize(UButton* InputButton, UTextBlock* InputText, float Size, FVector2D FontSize)
{
	FSlateFontInfo SlateFontInfo = CreateText->GetFont();
	SlateFontInfo.Size = Size;	
	InputText->SetFont(SlateFontInfo);
	FSlateBrush SlateBrush;
	SlateBrush.ImageSize = FontSize;
	FButtonStyle ButtonStyle = InputButton->GetStyle();
	ButtonStyle.SetPressed(SlateBrush);
}

void UMenu::ButtonStyle(UBorder* InputBorder, UButton* InputButton, UTextBlock* InputText, EButtonState NewButtonState)
{
	switch (NewButtonState)
	{
	case EButtonState::EBS_Pressed:
		if (InputBorder == nullptr) return ButtonSize(InputButton, InputText, 28.f, FVector2D(39.f, 39.f));
		else
		{
			InputBorder->SetVisibility(ESlateVisibility::Visible);
			ButtonSize(InputButton, InputText, 28.f, FVector2D(39.f, 39.f));
		}
		break;
	case EButtonState::EBS_Hovered:
		if(InputBorder == nullptr) return ButtonSize(InputButton, InputText, 28.f, FVector2D(39.f, 39.f));
		else
		{
			InputBorder->SetVisibility(ESlateVisibility::Visible);
			ButtonSize(InputButton, InputText, 28.f, FVector2D(39.f, 39.f));
		}
		break;
	case EButtonState::EBS_Unhovered:
		if (InputBorder == nullptr) return ButtonSize(InputButton, InputText, 25.f, FVector2D(32.f, 32.f));
		else
		{
			InputBorder->SetVisibility(ESlateVisibility::Hidden);
			ButtonSize(InputButton, InputText, 25.f, FVector2D(32.f, 32.f));
		}
		ButtonEnabled();
		break;
	case EButtonState::EBS_Released:
		if (InputBorder == nullptr) return ButtonSize(InputButton, InputText, 25.f, FVector2D(32.f, 32.f));
		else
		{
			InputBorder->SetVisibility(ESlateVisibility::Hidden);
			ButtonSize(InputButton, InputText, 25.f, FVector2D(32.f, 32.f));
		}
		break;
	case EButtonState::EBS_Disabled:
		if (InputBorder == nullptr) return ButtonSize(InputButton, InputText, 28.f, FVector2D(39.f, 39.f));
		else
		{
			InputBorder->SetVisibility(ESlateVisibility::Visible);
			ButtonSize(InputButton, InputText, 28.f, FVector2D(39.f, 39.f));
		}
		break;
	case EButtonState::EBS_Max:
		break;
	default:
		break;
	}
}

void UMenu::ButtonEnabled()
{
	if (!CreateButton->GetIsEnabled())
	{
		ButtonSize(CreateButton, CreateText, 28.f, FVector2D(39.f, 39.f));
		CreateHeightLight->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		CreateHeightLight->SetVisibility(ESlateVisibility::Hidden);
		ButtonSize(CreateButton, CreateText, 25.f, FVector2D(32.f, 32.f));
	}
	if (!JoinButton->GetIsEnabled())
	{
		ButtonSize(JoinButton, JoinText, 28.f, FVector2D(39.f, 39.f));
		JoinHeightLight->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		JoinHeightLight->SetVisibility(ESlateVisibility::Hidden);
		ButtonSize(JoinButton, JoinText, 25.f, FVector2D(32.f, 32.f));
	}
}

void UMenu::OnClickedCreate()
{
	JoinButton->SetIsEnabled(true);
	CreateButton->SetIsEnabled(false);
	ButtonStyle(CreateHeightLight, CreateButton, CreateText, EButtonState::EBS_Pressed);
	CreateAndJoin->SetActiveWidgetIndex(0);
}

void UMenu::OnReleasedJoin()
{
	JoinButton->SetIsEnabled(false);
	JoinButton->SetIsEnabled(true);
	ButtonStyle(JoinHeightLight, JoinButton, JoinText, EButtonState::EBS_Released);
}

void UMenu::OnClickedJoin()
{
	if (PlayerNumberHintAnimation)
	{
		if (IsAnimationPlaying(PlayerNumberHintAnimation))
		{
			StopAnimation(PlayerNumberHintAnimation);
			PlayerNumberText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	CreateButton->SetIsEnabled(true);
	JoinButton->SetIsEnabled(false);
	ButtonStyle(JoinHeightLight, JoinButton, JoinText, EButtonState::EBS_Pressed);
	CreateAndJoin->SetActiveWidgetIndex(1);
}

void UMenu::OnAddPeople()
{
	PlayerNumber += 1;

	if (PlayerNumber > 8)
	{
		if (PlayerNumberHintAnimation)
		{
			PlayerNumberText->SetVisibility(ESlateVisibility::Visible);
			PlayAnimation(PlayerNumberHintAnimation);
		}
		PlayerNumber = 8;
		FString NewIntString = FString::FromInt(PlayerNumber);
		InputPeopleNumber->SetText(FText::FromString(NewIntString));
	}
	FString IntString = FString::FromInt(PlayerNumber);
	InputPeopleNumber->SetText(FText::FromString(IntString));
}

void UMenu::OnSubPeople()
{
	PlayerNumber -= 1;

	if (PlayerNumber < 2)
	{
		if (PlayerNumberHintAnimation)
		{
			PlayerNumberText->SetVisibility(ESlateVisibility::Visible);
			PlayAnimation(PlayerNumberHintAnimation);
		}
		PlayerNumber = 2;
		FString NewIntString = FString::FromInt(PlayerNumber);
		InputPeopleNumber->SetText(FText::FromString(NewIntString));
	}
	FString IntString = FString::FromInt(PlayerNumber);
	InputPeopleNumber->SetText(FText::FromString(IntString));
}

void UMenu::OnInputPeopleNumberChanged(const FText& Text)
{
	FString InputString = Text.ToString();
	int32 NewPeopleNumber = FCString::Atoi(*InputString);
	if (InputString.Len() > 1 && NewPeopleNumber < 2)
	{
		if (PlayerNumberHintAnimation)
		{
			PlayerNumberText->SetVisibility(ESlateVisibility::Visible);
			PlayAnimation(PlayerNumberHintAnimation);
			PlayerNumber = NewPeopleNumber;
			FString IntString = FString::FromInt(PlayerNumber);
			InputPeopleNumber->SetText(FText::FromString(IntString));
		}
	}
	else
	{
		if (NewPeopleNumber < 9)
		{
			PlayerNumber = NewPeopleNumber;
			FString IntString = FString::FromInt(PlayerNumber);
			InputPeopleNumber->SetText(FText::FromString(IntString));
		}
	}
}

void UMenu::OnInputPeopleNumberCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString InputString = Text.ToString();
}

void UMenu::OnClickedSearchGame()
{
	SearchGameButton->SetIsEnabled(false);
	SearchGameText->SetText(FText::FromString(TEXT("搜索中")));
	if (SearchCircularThrobber)
	{
		SearchCircularThrobber->SetVisibility(ESlateVisibility::Visible);

		MultiplayerSessionsSubsystem->FindSession(10000);
	}
}

void UMenu::OnFindRoom(const TArray<FRoomInformation>& Rooms)
{
	GameRoomPlan->ClearChildren();

	SelectedRoomIndex = INDEX_NONE;
	LastSelectedRoom = nullptr;

	SearchGameButton->SetIsEnabled(true);
	SearchGameText->SetText(FText::FromString(TEXT("搜索游戏")));

	if (JoinGameButton)
	{
		JoinGameButton->SetIsEnabled(false);
	}

	if (SearchCircularThrobber)
	{
		SearchCircularThrobber->SetVisibility(ESlateVisibility::Hidden);
	}

	for (int32 i = 0; i < Rooms.Num(); ++i)
	{
		URoom* RoomWidget = CreateWidget<URoom>(this, RoomClass);
		RoomWidget->Init(Rooms[i], i);
		RoomWidget->OnRoomClicked.AddDynamic(this, &UMenu::HandleRoomSelected);

		GameRoomPlan->AddChild(RoomWidget);
	}
}

void UMenu::HandleRoomSelected(URoom* ClickedRoom)
{
	if (!ClickedRoom || ClickedRoom == LastSelectedRoom) return;

	//恢复上一个房间的按钮（启用）
	if (LastSelectedRoom)
	{
		LastSelectedRoom->SetSelectedState(false);
	}

	//禁用当前选中的房间按钮
	ClickedRoom->SetSelectedState(true);

	//更新 Menu 的记录
	LastSelectedRoom = ClickedRoom;

	//更新选中房间的索引（这一行很重要！）
    SelectedRoomIndex = ClickedRoom->GetIndex();

	//启用加入按钮
	if (JoinGameButton)
	{
		JoinGameButton->SetIsEnabled(true);
	}
}

//创建的房间名称更新
void UMenu::OnCreateRoomNameTextChanged(const FText& Text)
{
	FString InString = Text.ToString();

	FEditableTextBoxStyle NewStyle = CreateRoomNameText->WidgetStyle;
	NewStyle.TextStyle.SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f));
	CreateRoomNameText->WidgetStyle = NewStyle;
	CreateRoomNameText->SynchronizeProperties();
}

//创建的房间名称提交
void UMenu::OnCreateRoomNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString RoomNameString = Text.ToString();

	if (RoomNameString.Len() > 12)
	{
		RoomHintText->SetVisibility(ESlateVisibility::Visible);
		RoomHintText->SetText(FText::FromString(TEXT("房间名称不得超过12字符！！！")));
		PlayAnimation(RoomHintAnimation);
		CreateRoomNameText->SetText(FText::FromString(RoomNameString));
		FEditableTextBoxStyle Style = CreateRoomNameText->WidgetStyle;
		Style.TextStyle.SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f));
		CreateRoomNameText->WidgetStyle = Style;
		CreateRoomNameText->SynchronizeProperties();
	}
	else if (RoomNameString.Len() < 12)
	{
		if (RoomNameString.Contains(TEXT(" ")))
		{
			RoomHintText->SetVisibility(ESlateVisibility::Visible);
			RoomHintText->SetText(FText::FromString(TEXT("名称中不得有空白！！！")));
			PlayAnimation(RoomHintAnimation);
			return;
		}
		if (IsAnimationPlaying(RoomHintAnimation))
		{
			RoomHintText->SetVisibility(ESlateVisibility::Hidden);
		}
		CreateRoomNameText->SetText(FText::FromString(*RoomNameString));
		CreateRoomName = RoomNameString;
	}
}

void UMenu::CreateButtonClicked()
{
	ButtonStyle(nullptr, JoinGameButton, JoinGameText, EButtonState::EBS_Pressed);

	const UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UMultiplayerSessionsSubsystem* MSSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>();

		MSSubsystem->SetCachedPlayerCount(PlayerNumber);
	}

	if (CreateRoomName.IsEmpty())
	{
		RoomHintText->SetVisibility(ESlateVisibility::Visible);
		RoomHintText->SetText(FText::FromString(TEXT("房间名称不能为空")));
		PlayAnimation(RoomHintAnimation);
		CreateGameButton->SetIsEnabled(true);
		return;
	}

	CreateGameButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		BlockPlayerOverlay->SetVisibility(ESlateVisibility::Visible);
		LoadingWidget->SetVisibility(ESlateVisibility::Visible);
		MultiplayerSessionsSubsystem->CreateSession(CreateRoomName, PlayerNumber, MatchType);

	}
	else
	{
		BlockPlayerOverlay->SetVisibility(ESlateVisibility::Hidden);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("创建房间失败")));
	}
}

void UMenu::JoinButtonClicked()
{
	JoinGameButton->SetIsEnabled(false);

	if(SelectedRoomIndex == INDEX_NONE)
	{
		JoinGameButton->SetIsEnabled(true);
		return;
	}

	if (!MultiplayerSessionsSubsystem || !MultiplayerSessionsSubsystem->LastSearchResults.IsValidIndex(SelectedRoomIndex))
	{
		JoinGameButton->SetIsEnabled(true);
		BlockPlayerOverlay->SetVisibility(ESlateVisibility::Hidden);
		LoadingWidget->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const FOnlineSessionSearchResult& Result = MultiplayerSessionsSubsystem->LastSearchResults[SelectedRoomIndex];

	if (MultiplayerSessionsSubsystem)
	{
		BlockPlayerOverlay->SetVisibility(ESlateVisibility::Visible);
		LoadingWidget->SetVisibility(ESlateVisibility::Visible);
		MultiplayerSessionsSubsystem->JoinSession(Result);
	}
	else
	{
		BlockPlayerOverlay->SetVisibility(ESlateVisibility::Hidden);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("加入房间失败房间失败")));
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();//从父项中移除
	UWorld* World = GetWorld();
	if (World)
	{
		//获取首个玩家控制器
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;//仅游戏模式输入
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(PathToHall);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{

	if (!bWasSuccessful) return;

	MultiplayerSessionsSubsystem->LastSearchResults = SessionResult;
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinGameButton->SetIsEnabled(true);
		return;

	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			bool bGotAddress = SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			if (bGotAddress || !Address.IsEmpty())
			{
				APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
				if (PlayerController)
				{
					PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
}
