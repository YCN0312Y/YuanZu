#include "UI/Lobby/YuanZuTeam.h"
#include "Gameplay/Lobby/YuanZuLobbyPlayerController.h"
#include "Gameplay/Lobby/YuanZuLobbyGameState.h"
#include "UI/Lobby/YuanZuTeamPlayer.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Components/VerticalBox.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/YuanZuTeamCharacter.h"

bool UYuanZuTeam::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UYuanZuTeam::NativeConstruct()
{
	Super::NativeConstruct();

	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS) return;

	LobbyGS->OnTeamSlotsChanged.RemoveAll(this);
	LobbyGS->OnTeamSlotsChanged.AddUObject(this, &UYuanZuTeam::RefreshAllSlots);

	const TArray<FYuanZuTeamSlotInfo>& TeamSlots = LobbyGS->GetTeamSlots();

	CreateTeamPlayerFromSlots(TeamSlots);
	RefreshAllSlots(TeamSlots);
}

void UYuanZuTeam::NativeDestruct()
{
	if (CachedLobbyGameState)
	{
		CachedLobbyGameState->OnTeamSlotsChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UYuanZuTeam::CreateTeamPlayerFromSlots(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots)
{
	if (!RedTeam || !BlueTeam || !TeamPlayerClass) return;

	RedTeam->ClearChildren();
	BlueTeam->ClearChildren();
	RedTeamPlayers.Empty();
	BlueTeamPlayers.Empty();

	for (const FYuanZuTeamSlotInfo& SlotInfo : InTeamSlots)
	{
		if (SlotInfo.TeamType == ETeamType::ETT_Red)
		{
			UYuanZuTeamPlayer* RedTeamPlayer = CreateWidget<UYuanZuTeamPlayer>(GetOwningPlayer(), TeamPlayerClass);
			if (RedTeamPlayer)
			{
				RedTeamPlayer->SetPadding(FMargin(0.f, 0.f, 0.f, 5.f));
				RedTeamPlayer->InitSlotInfo(ETeamType::ETT_Red, SlotInfo.SlotIndex);
				RedTeam->AddChild(RedTeamPlayer);
				RedTeamPlayers.Add(RedTeamPlayer);
			}
		}
		else if (SlotInfo.TeamType == ETeamType::ETT_Blue)
		{
			UYuanZuTeamPlayer* BlueTeamPlayer = CreateWidget<UYuanZuTeamPlayer>(GetOwningPlayer(), TeamPlayerClass);
			if (BlueTeamPlayer)
			{
				BlueTeamPlayer->SetPadding(FMargin(0.f, 0.f, 0.f, 5.f));
				BlueTeamPlayer->InitSlotInfo(ETeamType::ETT_Blue, SlotInfo.SlotIndex);
				BlueTeam->AddChild(BlueTeamPlayer);
				BlueTeamPlayers.Add(BlueTeamPlayer);
			}
		}
	}

}

void UYuanZuTeam::RefreshAllSlots(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots)
{
	int32 RedSlotCount = 0;
	int32 BlueSlotCount = 0;

	//根据插槽数量循环
	for (const FYuanZuTeamSlotInfo& SlotInfo : InTeamSlots)
	{
		if (SlotInfo.TeamType == ETeamType::ETT_Red)
		{
			RedSlotCount++;
		}
		else if (SlotInfo.TeamType == ETeamType::ETT_Blue)
		{
			BlueSlotCount++;
		}
	}

	if (RedTeamPlayers.Num() != RedSlotCount || BlueTeamPlayers.Num() != BlueSlotCount)
	{
		CreateTeamPlayerFromSlots(InTeamSlots);
	}

	for (const FYuanZuTeamSlotInfo& SlotInfo : InTeamSlots)
	{
		if (SlotInfo.TeamType == ETeamType::ETT_Red)
		{
			//检查插槽索引是否有效没越界和这个插槽是不是空指针
			if (RedTeamPlayers.IsValidIndex(SlotInfo.SlotIndex) && RedTeamPlayers[SlotInfo.SlotIndex])
			{
				//再将该索引的插槽数据传给刷新插槽
				RedTeamPlayers[SlotInfo.SlotIndex]->RefreshSlot(SlotInfo);
			}
		}
		else if (SlotInfo.TeamType == ETeamType::ETT_Blue)
		{
			if (BlueTeamPlayers.IsValidIndex(SlotInfo.SlotIndex) && BlueTeamPlayers[SlotInfo.SlotIndex])
			{
				BlueTeamPlayers[SlotInfo.SlotIndex]->RefreshSlot(SlotInfo);
			}
		}
	}

	UpdateTeamCharacterMI(InTeamSlots);
}

void UYuanZuTeam::UpdateTeamCharacterMI(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots)
{
	if (!CachedTeamCharacter)
	{
		TArray<AActor*> FoundActors;
		//获取世界中所有的AActor类
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AYuanZuTeamCharacter::StaticClass(), FoundActors);

		if (FoundActors.Num() > 0)
		{
			//如果Actor的数量 > 0就转换成缓存的阵容角色
			CachedTeamCharacter = Cast<AYuanZuTeamCharacter>(FoundActors[0]);
		}
	}

	AYuanZuLobbyPlayerController* YuanZuLPC = GetOwningPlayer<AYuanZuLobbyPlayerController>();
	if (!YuanZuLPC) return;

	APlayerState* PS = YuanZuLPC->GetPlayerState<APlayerState>();
	if (!PS)
	{
		CachedTeamCharacter->SetTeamCharacter(ETeamType::ETT_None);
		return;
	}

	ETeamType MyTeamType = ETeamType::ETT_None;

	for (const FYuanZuTeamSlotInfo& SlotInfo : InTeamSlots)
	{
		if (SlotInfo.OccupyingPlayer == PS)
		{
			MyTeamType = SlotInfo.TeamType;
			break;
		}
	}

	CachedTeamCharacter->SetTeamCharacter(MyTeamType);
}
