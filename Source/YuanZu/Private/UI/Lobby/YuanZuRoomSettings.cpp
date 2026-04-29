#include "UI/Lobby/YuanZuRoomSettings.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Gameplay/Lobby/YuanZuLobbyGameState.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerState.h"
#include "UI/Lobby/Rests/YuanZuMapOptionData.h"
#include "GameFramework/PlayerState.h"

void UYuanZuRoomSettings::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UYuanZuRoomSettings::OnClickedStartGameButton);
	}
	if (MapComboBox)
	{
		MapComboBox->OnSelectionChanged.AddDynamic(this, &UYuanZuRoomSettings::OnSelectionChangedMap);
	}
	if (QuitTeamButton)
	{
		QuitTeamButton->OnClicked.AddDynamic(this, &UYuanZuRoomSettings::OnClickedQuitTeamButton);
	}
}

void UYuanZuRoomSettings::NativePreConstruct()
{
	Super::NativePreConstruct();

	QuitTeamButton->SetIsEnabled(false);
}

void UYuanZuRoomSettings::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateMapCount();
	UpdateRoomSettingsAuthorityUI();

	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (LobbyGS)
	{
		LobbyGS->OnSelectedMapChanged.RemoveAll(this);
		LobbyGS->OnSelectedMapChanged.AddUObject(this, &UYuanZuRoomSettings::RefreshSelectedMapText);
		LobbyGS->OnTeamSlotsChanged.RemoveAll(this);
		LobbyGS->OnTeamSlotsChanged.AddUObject(this, &UYuanZuRoomSettings::HandleTeamSlotsChanged);
		LobbyGS->OnTeamSlotsChanged.AddUObject(this, &UYuanZuRoomSettings::UpdateQuitTeamButtonState);
		RefreshSelectedMapText(LobbyGS->GetSelectedMapName());
	}

	RefreshStartGameButtonState();
}

void UYuanZuRoomSettings::OnSelectionChangedMap(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!LPC || !LPC->HasAuthority())
	{
		return;
	}

	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS) return;

	LobbyGS->SetSelectedMapName(SelectedItem);
	UpdateMapPathByDisplayName(SelectedItem);
}

void UYuanZuRoomSettings::OnClickedStartGameButton()
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!LPC) return;

	const bool bIsHost = LPC->HasAuthority();
	if (bIsHost)
	{
		FString SelectedMapName;
		AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
		if (LobbyGS)
		{
			SelectedMapName = LobbyGS->GetSelectedMapName();
		}

		if (SelectedMapName.IsEmpty() && MapComboBox)
		{
			SelectedMapName = MapComboBox->GetSelectedOption();
		}

		if (!UpdateMapPathByDisplayName(SelectedMapName)) return;
		if (MapPath.IsEmpty()) return;
		if (!AreAllOccupiedPlayersReady()) return;

		if (StartGameButton)
		{
			StartGameButton->SetIsEnabled(false);
		}

		// 房主点击后走服务端统一开局流程。
		LPC->ServerRequestStartGame(MapPath);
		return;
	}

	bool bIsReady = false;
	if (!IsLocalPlayerInTeam(bIsReady)) return;

	// 房员按钮在“准备”和“取消准备”之间切换。
	LPC->ServerSetReadyState(!bIsReady);
}

void UYuanZuRoomSettings::OnClickedQuitTeamButton()
{
	AYuanZuLobbyPlayerController* YuanzuLPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!YuanzuLPC)return;

	YuanzuLPC->ServerClearSlot();
}

void UYuanZuRoomSettings::RefreshSelectedMapText(const FString& InMapDisplayName)
{
	if (MapNameText)
	{
		MapNameText->SetText(FText::FromString(InMapDisplayName));
	}
}

void UYuanZuRoomSettings::HandleTeamSlotsChanged(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots)
{
	RefreshStartGameButtonState();
}

//刷新开始游戏按键状态
void UYuanZuRoomSettings::RefreshStartGameButtonState()
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!LPC) return;

	const bool bIsHost = LPC->HasAuthority();
	bool bLocalReady = false;
	const bool bLocalPlayerInTeam = IsLocalPlayerInTeam(bLocalReady);

	if (StartGameText)
	{
		if (bIsHost)
		{
			//房主始终显示开始游戏
			StartGameText->SetText(FText::FromString(TEXT("开始游戏")));
		}
		else
		{
			//房员根据自己的准备状态显示准备/取消准备
			StartGameText->SetText(FText::FromString(bLocalReady ? TEXT("取消准备") : TEXT("准备")));
		}
	}

	if (StartGameButton)
	{
		if (bIsHost)
		{
			// 房主按钮只有在所有已入队玩家都准备后才可点击。
			StartGameButton->SetIsEnabled(AreAllOccupiedPlayersReady());
		}
		else
		{
			StartGameButton->SetIsEnabled(bLocalPlayerInTeam);
		}
	}
}

void UYuanZuRoomSettings::UpdateQuitTeamButtonState(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots)
{
	AYuanZuLobbyPlayerController* YuanZuLPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!YuanZuLPC)return;

	AYuanZuLobbyPlayerState* YuanZuLPS = YuanZuLPC->GetPlayerState<AYuanZuLobbyPlayerState>();
	if (!YuanZuLPS)return;

	if (!GetWorld())return;

	AYuanZuLobbyGameState* YuanZuLGS = GetWorld()->GetGameState<AYuanZuLobbyGameState>();
	if (!YuanZuLGS)return;

	if (YuanZuLGS->IsPlayerTeam(YuanZuLPS))
	{
		QuitTeamButton->SetIsEnabled(true);
	}
	else
	{
		QuitTeamButton->SetIsEnabled(false);
	}
}

void UYuanZuRoomSettings::UpdateRoomSettingsAuthorityUI()
{
	AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	const bool bIsHost = LPC && LPC->HasAuthority();

	if (MapComboBox)
	{
		MapComboBox->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		MapComboBox->SetIsEnabled(bIsHost);
	}

	if (StartGameButton)
	{
		StartGameButton->SetVisibility(ESlateVisibility::Visible);
	}

	if (MapNameTextBorder)
	{
		MapNameTextBorder->SetVisibility(bIsHost ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (MapNameText)
	{
		MapNameText->SetVisibility(bIsHost ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		if (!bIsHost && MapComboBox)
		{
			MapNameText->SetText(FText::FromString(MapComboBox->GetSelectedOption()));
		}
	}
}

void UYuanZuRoomSettings::UpdateMapCount()
{
	if (!MapComboBox) return;

	MapComboBox->ClearOptions();

	for (const FYuanZuMapOptionData& MapDataTmp : MapData)
	{
		if (!MapDataTmp.MapDisplayName.IsEmpty())
		{
			MapComboBox->AddOption(MapDataTmp.MapDisplayName);
		}
	}

	if (MapData.Num() > 0)
	{
		MapComboBox->SetSelectedOption(MapData[0].MapDisplayName);
		UpdateMapPathByDisplayName(MapData[0].MapDisplayName);
	}

	MapComboBox->RefreshOptions();
}

bool UYuanZuRoomSettings::UpdateMapPathByDisplayName(const FString& InSelectedMapName)
{
	if (InSelectedMapName.IsEmpty()) return false;

	for (const FYuanZuMapOptionData& MapDataTmp : MapData)
	{
		if (MapDataTmp.MapDisplayName == InSelectedMapName)
		{
			MapPath = MapDataTmp.MapPath;
			return !MapPath.IsEmpty();
		}
	}

	MapPath.Empty();
	return false;
}

bool UYuanZuRoomSettings::IsLocalPlayerInTeam(bool& bOutReady) const
{
	bOutReady = false;

	const AYuanZuLobbyPlayerController* LPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	const APlayerState* LocalPS = LPC ? LPC->GetPlayerState<APlayerState>() : nullptr;
	if (!LocalPS) return false;

	const AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS) return false;

	for (const FYuanZuTeamSlotInfo& SlotInfo : LobbyGS->GetTeamSlots())
	{
		if (SlotInfo.OccupyingPlayer == LocalPS)
		{
			// 本地按钮文案直接读取自己所在槽位的准备状态。
			bOutReady = SlotInfo.bReady;
			return true;
		}
	}

	return false;
}

bool UYuanZuRoomSettings::AreAllOccupiedPlayersReady() const
{
	const AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS) return false;

	bool bHasOccupiedPlayer = false;

	for (const FYuanZuTeamSlotInfo& SlotInfo : LobbyGS->GetTeamSlots())
	{
		if (!SlotInfo.bOccupied)
		{
			continue;
		}

		bHasOccupiedPlayer = true;
		if (!SlotInfo.bReady)
		{
			// 任意一个已入队玩家未准备，房主都不能开始游戏。
			return false;
		}
	}

	return bHasOccupiedPlayer;
}
