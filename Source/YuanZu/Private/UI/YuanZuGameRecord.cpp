// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/YuanZuGameRecord.h"
#include "UI/YuanZuPlayerRecord.h"
#include "Components/VerticalBox.h"
#include "Gameplay/YuanZuPlayerState.h"
#include "Gameplay/YuanZuPlayerController.h"
#include "Gameplay/YuanZuGameState.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"

void UYuanZuGameRecord::RefreshRecordList()
{
	if (!RedTeam || !BlueTeam)return;
	//先清楚所有子类
	RedTeam->ClearChildren();
	BlueTeam->ClearChildren();

	if (!GetWorld())return;

	AGameState* GS = GetWorld()->GetGameState<AGameState>();
	if (!GS)return;
	for (APlayerState* PS : GS->PlayerArray)
	{
		AYuanZuPlayerState* YuanZuPS = Cast<AYuanZuPlayerState>(PS);
		if (!YuanZuPS)continue;

		UYuanZuPlayerRecord* YuanZuPR = CreateWidget<UYuanZuPlayerRecord>(GetOwningPlayer(), PlayerRecord);
		//初始化玩家战绩面板
		if (YuanZuPR)
		{
			FString TargetPlayerName = YuanZuPS->GetPlayerNameCustom();//玩家名称
			float TargetPlayerKillScore = YuanZuPS->GetScore();//击杀分数
			int32 TargetPlayerDeathScore = YuanZuPS->GetDeathScore();//死亡分数
			int32 TargetPlayerSecondaryScore = YuanZuPS->GetSecondaryScore();//助攻分数
			ETeamType TargetPlayerTeam = YuanZuPS->GetTeamType();//所属阵营
			YuanZuPR->InitPlayerRecord(TargetPlayerTeam, TargetPlayerName, TargetPlayerKillScore, TargetPlayerDeathScore, TargetPlayerSecondaryScore);
		}

		//根据玩家所选设置阵容
		switch (YuanZuPS->GetTeamType())
		{
		case ETeamType::ETT_Red:
			RedTeam->AddChild(YuanZuPR);
			break;
		case ETeamType::ETT_Blue:
			BlueTeam->AddChild(YuanZuPR);
			break;
		}
	}
}
