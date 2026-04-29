// Fill out your copyright notice in the Description page of Project Settings.
#include "Gameplay/YuanZuGameMode.h"
#include "Character/YuanZuCharacterBase.h"
#include "Gameplay/YuanZuPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Gameplay/YuanZuPlayerState.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Gameplay/YuanZuGameInstance.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AYuanZuGameMode::AYuanZuGameMode()
{
	//延迟开始
	bDelayedStart = true;
}

void AYuanZuGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AYuanZuGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AYuanZuPlayerController* YuanZuPlayerController = Cast<AYuanZuPlayerController>(*It);
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->OnMatchStateSet(GetMatchState());
		}
	}
}

void AYuanZuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	SyncSessionPlayerCount();

	AYuanZuPlayerState* YuanZuPS = NewPlayer ? NewPlayer->GetPlayerState<AYuanZuPlayerState>() : nullptr;

	UYuanZuGameInstance* YuanZuGI = GetGameInstance<UYuanZuGameInstance>();

	if (!YuanZuPS || !YuanZuGI || !YuanZuPS->GetUniqueId().IsValid())return;

	//玩家ID
	const FString PlayerNetID = YuanZuPS->GetUniqueId().ToString();

	ETeamType CacheTeamType = ETeamType::ETT_None;
	if (YuanZuGI->GetPlayerTeams(PlayerNetID, CacheTeamType))
	{
		YuanZuPS->SetPlayerTeam(CacheTeamType);
	}

}

void AYuanZuGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	SyncSessionPlayerCount();
}

void AYuanZuGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//比赛状态等待开始
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		//在等待开始游戏时：倒计时 = 总热身时间 - (当前时间 - 关卡开始时间) 关卡开始时间：客户端玩家加入的时间
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.0f)
		{
			StartMatch();
		}
	}
	else if (GetMatchState() == MatchState::InProgress)
	{
		//在比赛进行中：倒计时 = 热身时间 + 比赛时间 - 当前时间 + 关卡开始时间
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.0f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (GetMatchState() == MatchState::Cooldown)
	{
		//在游戏结束时：倒计时 = 冷却时间 + 热身时间 + 比赛时间 - 当前时间 + 关卡开始时间
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.0f)
		{
			//重新开始游戏
			RestartGame();
		}
	}
}

void AYuanZuGameMode::PlayerDeath(AYuanZuCharacterBase* DeathCharacter, AYuanZuPlayerController* DeathController, AYuanZuPlayerController* AttackerController)
{
	/*
	DeathCharacter受害者角色
	DeathController受害者控制器
	AttackerController攻击者控制器
	*/

	//攻击者状态
	AYuanZuPlayerState* AttackerPlayerState = AttackerController ? Cast<AYuanZuPlayerState>(AttackerController->PlayerState) : nullptr;
	//受害者状态
	AYuanZuPlayerState* DeathPlayerState = DeathController ? Cast<AYuanZuPlayerState>(DeathController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != DeathPlayerState)
	{
		AttackerPlayerState->AddToKillScore(1.f);
	}
	if (DeathPlayerState)
	{
		DeathPlayerState->AddToDeathScore(1);
	}
	if (DeathCharacter)
	{
		DeathCharacter->Death();

	}
}

void AYuanZuGameMode::RequestRespawn(ACharacter* DeathCharacter, AController* DeathController)
{
	if (DeathCharacter)
	{
		DeathCharacter->Reset();//重置角色
		DeathCharacter->Destroy();//摧毁角色
	}
	if (DeathController)
	{
		TArray<AActor*> PlayerStatrs;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStatrs);
		int32 Selection = FMath::RandRange(0, PlayerStatrs.Num() - 1);
		RestartPlayerAtPlayerStart(DeathController, PlayerStatrs[Selection]);//在玩家开始位置重新生成玩家
	}
}

void AYuanZuGameMode::HandleHostQuitToLogin()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AYuanZuPlayerController* YuanZuPC = Cast<AYuanZuPlayerController>(It->Get());
		if (!YuanZuPC)
		{
			continue;
		}

		//房主本机直接执行
		if (YuanZuPC->IsLocalController())
		{
			YuanZuPC->BeginReturnToLogin();
		}
		else
		{
			//远端客户端通过回登录
			YuanZuPC->ClientReturnToLogin();
		}
	}
}

//同步会话玩家数量
void AYuanZuGameMode::SyncSessionPlayerCount()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UMultiplayerSessionsSubsystem* Subsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if (!Subsystem) return;

	const int32 CurrentPlayers = GetNumPlayers();
	Subsystem->UpdateCurrentPlayerCount(CurrentPlayers);
}


