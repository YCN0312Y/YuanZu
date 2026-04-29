// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuPlayerState.generated.h"

class AYuanZuCharacterBase;
class AYuanZuPlayerController;

UCLASS()
class YUANZU_API AYuanZuPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

private:
	UPROPERTY()
	AYuanZuCharacterBase* YuanZuCharacter;
	UPROPERTY()
	AYuanZuPlayerController* YuanZuPlayerController;

	//玩家名称
	UPROPERTY(ReplicatedUsing = OnRep_PlayerNameCustom)
	FString PlayerNameCustom;
	//死亡分数
	UPROPERTY(ReplicatedUsing = OnRep_DeathScore)
	int32 DeathScore;
	//助攻分数
	int32 SecondaryScore;
	//所属阵营
	UPROPERTY(ReplicatedUsing = OnRep_TeamType)
	ETeamType TeamType = ETeamType::ETT_None;

public:
	//死亡分数
	UFUNCTION()
	virtual void OnRep_DeathScore();
	//玩家名称
	UFUNCTION()
	void OnRep_PlayerNameCustom();
	//玩家阵营
	UFUNCTION()
	void OnRep_TeamType();

	//击杀分数
	void AddToKillScore(float ScoreAmount);
	//死亡次数
	void AddToDeathScore(int32 DeathAmount);
	//设置玩家名称
	void SetPlayerNameCustom(const FString& NewName);
	//设置玩家阵营
	void SetPlayerTeam(ETeamType NewTeamType);

	FORCEINLINE int32 GetDeathScore()const { return DeathScore; }
	FORCEINLINE int32 GetSecondaryScore()const { return SecondaryScore; }
	FORCEINLINE FString GetPlayerNameCustom()const { return PlayerNameCustom; }
	FORCEINLINE ETeamType GetTeamType()const { return TeamType; }
};
