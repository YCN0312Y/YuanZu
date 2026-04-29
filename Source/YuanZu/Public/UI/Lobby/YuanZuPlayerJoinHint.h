// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Lobby/Rests/YuanZuLobbyHintInfo.h"
#include "YuanZuPlayerJoinHint.generated.h"

class UScrollBox;
class UYuanZuPlayerStateHint;
class AYuanZuLobbyGameState;

UCLASS()
class YUANZU_API UYuanZuPlayerJoinHint : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	//提示信息栏
	UPROPERTY(meta = (BindWidget))
	UScrollBox* HintScrollBox;
	//大厅玩家状态类
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuPlayerStateHint> PlayerStateHintClass;

	UPROPERTY()
	TObjectPtr<AYuanZuLobbyGameState> CachedLobbyGameState = nullptr;

	// 如果控件创建时 GameState 还没准备好，就延迟重试绑定
	FTimerHandle BindLobbyGameStateTimerHandle;

private:
	// 找到大厅 GameState 并绑定提示变化委托
	void TryBindLobbyGameState();
	// 根据最新提示数组重建右侧提示列表
	void RefreshHints(const TArray<FYuanZuLobbyHintInfo>& InHints);

};
