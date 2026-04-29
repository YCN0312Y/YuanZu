// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuTeam.generated.h"

class UVerticalBox;
class UYuanZuTeamPlayer;
class AYuanZuTeamCharacter;
class AYuanZuLobbyGameState;

struct FYuanZuTeamSlotInfo;

UCLASS()
class YUANZU_API UYuanZuTeam : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	//加入队伍的玩家类
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuTeamPlayer> TeamPlayerClass;
	//红队
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RedTeam;
	//蓝队
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* BlueTeam;

	//红队
	UPROPERTY()
	TArray<TObjectPtr<UYuanZuTeamPlayer>> RedTeamPlayers;
	//蓝队
	UPROPERTY()
	TArray<TObjectPtr<UYuanZuTeamPlayer>> BlueTeamPlayers;
	//缓存的大厅游戏状态
	UPROPERTY()
	TObjectPtr<AYuanZuLobbyGameState> CachedLobbyGameState;
	//缓存的阵容角色
	UPROPERTY()
	TObjectPtr<AYuanZuTeamCharacter> CachedTeamCharacter;

private:

	void CreateTeamPlayerFromSlots(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots);
	void RefreshAllSlots(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots);

	void UpdateTeamCharacterMI(const TArray<FYuanZuTeamSlotInfo>& InTeamSlots);
};
