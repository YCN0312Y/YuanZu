#include "UI/Lobby/YuanZuPlayerJoinHint.h"
#include "Components/ScrollBox.h"
#include "Gameplay/Lobby/YuanZuLobbyGameState.h"
#include "TimerManager.h"
#include "UI/Lobby/YuanZuPlayerStateHint.h"

void UYuanZuPlayerJoinHint::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UYuanZuPlayerJoinHint::NativeConstruct()
{
	Super::NativeConstruct();

	// 控件创建完成后开始寻找并绑定大厅 GameState
	TryBindLobbyGameState();
}

void UYuanZuPlayerJoinHint::NativeDestruct()
{
	if (CachedLobbyGameState)
	{
		CachedLobbyGameState->OnPlayerJoinHintsChanged.RemoveAll(this);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BindLobbyGameStateTimerHandle);
	}

	Super::NativeDestruct();
}

void UYuanZuPlayerJoinHint::TryBindLobbyGameState()
{
	AYuanZuLobbyGameState* LobbyGS = GetWorld() ? GetWorld()->GetGameState<AYuanZuLobbyGameState>() : nullptr;
	if (!LobbyGS)
	{
		if (GetWorld())
		{
			// 某些时机下 UI 比 GameState 更早创建，这里短暂延迟后再试一次
			GetWorld()->GetTimerManager().SetTimer(
				BindLobbyGameStateTimerHandle,
				this,
				&UYuanZuPlayerJoinHint::TryBindLobbyGameState,
				0.1f,
				false
			);
		}
		return;
	}

	CachedLobbyGameState = LobbyGS;
	CachedLobbyGameState->OnPlayerJoinHintsChanged.RemoveAll(this);
	// 绑定提示变化委托，这样任何一端收到复制更新后都会自动刷新列表
	CachedLobbyGameState->OnPlayerJoinHintsChanged.AddUObject(this, &UYuanZuPlayerJoinHint::RefreshHints);

	// 初次绑定成功后，立刻用当前已有数据刷新一次 UI
	RefreshHints(CachedLobbyGameState->GetPlayerJoinHints());
}

void UYuanZuPlayerJoinHint::RefreshHints(const TArray<FYuanZuLobbyHintInfo>& InHints)
{
	if (!HintScrollBox || !PlayerStateHintClass) return;

	// 这里采用“整体重建列表”的方式，逻辑更直接，适合当前这种提示数量较少的场景
	HintScrollBox->ClearChildren();

	for (const FYuanZuLobbyHintInfo& HintInfo : InHints)
	{
		UYuanZuPlayerStateHint* HintWidget = CreateWidget<UYuanZuPlayerStateHint>(GetOwningPlayer(), PlayerStateHintClass);
		if (!HintWidget) continue;

		HintWidget->SetHintInfo(HintInfo.PlayerName, HintInfo.StateText, HintInfo.TimeString);
		HintScrollBox->AddChild(HintWidget);
	}

	// 新提示出现后自动滚动到末尾，保证玩家能第一时间看到最新消息
	HintScrollBox->ScrollToEnd();
}
