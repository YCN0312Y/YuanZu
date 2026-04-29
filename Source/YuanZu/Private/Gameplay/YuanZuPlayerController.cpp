// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/YuanZuPlayerController.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/YuanZuCharacterBase.h"
#include "Components/YuanZuCombatComponent.h"
#include "Weapons/YuanZuWeapon.h"
#include "Gameplay/YuanZuHUD.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/YuanZuGameMode.h"
#include "Gameplay/YuanZuPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "YuanZu/Log/YuanZuLog.h"
#include "UI/YuanZuAnnouncement.h"
#include "UI/YuanZuCharacterOverlay.h"
#include "UI/YuanZuGameRecord.h"
#include "UI/YuanZuMainUIBase.h"
#include "UI/YuanZuOverHeadWidget.h"
#include "UI/YuanZuPlayerRecord.h"
#include "UI/Weapon/YuanZuWeaponProperty.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/PlayerInformation.h"
#include "UI/YuanZuGameSettings.h"
#include "MultiplayerSessionsSubsystem.h"

AYuanZuPlayerController::AYuanZuPlayerController()
{

}

void AYuanZuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;
	FInputModeGameOnly GameInputMode;
	SetInputMode(GameInputMode);

	YuanZuHUD = Cast<AYuanZuHUD>(GetHUD());
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMultiplayerSessionsSubsystem* MPSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
		{
			const FString CachedName = MPSubsystem->GetCachedPlayerName();
			if (!CachedName.IsEmpty())
			{
				UE_LOG(LogTemp, Warning, TEXT("BeginPlay send PlayerName = %s"), *CachedName);
				ServerSetPlayerNameFromUI(CachedName);
			}
		}
	}

	ServerCheckMatchState();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(YuanZuMappingContext, 0);
		}
	}
}

void AYuanZuPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYuanZuPlayerController, MatchState);
	DOREPLIFETIME(AYuanZuPlayerController, bDisableGameplay);

}

void AYuanZuPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);
}

void AYuanZuPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	//如果距离上次同步的时间 > 时间同步频率
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFreqyency)
	{
		//再向服务器请求同步
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.0f;
	}
}

void AYuanZuPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AYuanZuPlayerController::Move);//移动
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AYuanZuPlayerController::Look);//看
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AYuanZuPlayerController::Jump);//跳
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AYuanZuPlayerController::StopJumping);//停止跳
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered, this, &AYuanZuPlayerController::StartRunning);//跑
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AYuanZuPlayerController::StopRunning);//停止跑
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &AYuanZuPlayerController::EquipButtonPressed);//装备
		EnhancedInputComponent->BindAction(DroppedAction, ETriggerEvent::Started, this, &AYuanZuPlayerController::Dropped);//丢弃
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AYuanZuPlayerController::CrouchButtonPressed);//蹲下
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AYuanZuPlayerController::AimButtonPressed);//瞄准
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AYuanZuPlayerController::AimButtonReleased);//停止瞄准
		EnhancedInputComponent->BindAction(SwimAction, ETriggerEvent::Triggered, this, &AYuanZuPlayerController::Swim);//游泳
		EnhancedInputComponent->BindAction(SwimAction, ETriggerEvent::Completed, this, &AYuanZuPlayerController::StopSwimming);//停止游泳
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AYuanZuPlayerController::Fire);//开火
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AYuanZuPlayerController::StopFire);//停止开火
		EnhancedInputComponent->BindAction(ShowGameRecordAction, ETriggerEvent::Triggered, this, &AYuanZuPlayerController::ShowGameRecord);//显示局内战绩
		EnhancedInputComponent->BindAction(ShowGameRecordAction, ETriggerEvent::Completed, this, &AYuanZuPlayerController::HideGameRecord);//隐藏局内战绩
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AYuanZuPlayerController::Reload);//换弹
		EnhancedInputComponent->BindAction(SettingsAction, ETriggerEvent::Started, this, &AYuanZuPlayerController::ShowSetting);//设置
	}
}

void AYuanZuPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	YuanZuCharacter = Cast<AYuanZuCharacterBase>(GetPawn());
	if (YuanZuCharacter)
	{
		SetHealth(YuanZuCharacter->GetCurrentHealth(), YuanZuCharacter->GetMaxHealth());
	}
	AYuanZuPlayerState* YuanZuPS = GetPlayerState<AYuanZuPlayerState>();
	AYuanZuCharacterBase* YuanZuC = Cast<AYuanZuCharacterBase>(GetPawn());
	if (YuanZuPS && YuanZuC)
	{
		YuanZuC->SetCharacterMaterila(YuanZuPS->GetTeamType());
	}
}

void AYuanZuPlayerController::OnUnPossess()
{
	YuanZuCharacter = nullptr;

	Super::OnUnPossess();

}

void AYuanZuPlayerController::OnRep_Pawn()
{
	//网络复制让客户端在服务端将YuanZuCharacter变量Cast
	YuanZuCharacter = Cast<AYuanZuCharacterBase>(GetPawn());

	AYuanZuPlayerState* YuanZuPS = GetPlayerState<AYuanZuPlayerState>();
	if (YuanZuPS)
	{
		AYuanZuCharacterBase* YuanZuC = Cast<AYuanZuCharacterBase>(GetPawn());
		if (YuanZuPS && YuanZuC)
		{
			YuanZuC->SetCharacterMaterila(YuanZuPS->GetTeamType());
		}
		SetHUDPlayerName(YuanZuPS->GetPlayerNameCustom());
	}
}

void AYuanZuPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AYuanZuPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	UE_LOG(LogTemp, Warning, TEXT("MatchState:%s"), *MatchState.ToString());
	if (MatchState == MatchState::WaitingToStart)
	{
		YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
		if (YuanZuHUD)
		{
			YuanZuHUD->AddAnnouncement();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleInProgress();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

//比赛进行中
void AYuanZuPlayerController::HandleInProgress()
{
	//如果比赛状态变成进行中就创建控件
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
	if (YuanZuHUD)
	{
		YuanZuHUD->CreateMainUI();

		if (YuanZuHUD->Announcement)
		{
			YuanZuHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (YuanZuCharacter)
		{
			SetHealth(YuanZuCharacter->GetCurrentHealth(), YuanZuCharacter->GetMaxHealth());

			AYuanZuPlayerState* YuanZuPlayerState = Cast<AYuanZuPlayerState>(PlayerState);
			if (YuanZuPlayerState)
			{
				SetHUDPlayerName(YuanZuPlayerState->GetPlayerNameCustom());
				SetHUDDeathScore(YuanZuPlayerState->GetDeathScore());
				SetHUDKillScore(YuanZuPlayerState->GetScore());
				SetHUDPlayerIcon(YuanZuPlayerState->GetTeamType());
			}
		}
	}
}

//比赛冷却中
void AYuanZuPlayerController::HandleCooldown()
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;

	if (YuanZuHUD)
	{
		//移除
		YuanZuHUD->MainUIBase->RemoveFromParent();

		if (YuanZuHUD->Announcement && YuanZuHUD->Announcement->AnnouncementText)
		{
			YuanZuHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			YuanZuHUD->Announcement->AnnouncementText->SetText(FText::FromString(TEXT("新比赛即将开始")));
		}
	}

	bDisableGameplay = true;

	if (YuanZuCharacter && YuanZuCharacter->GetCombat() && YuanZuCharacter->GetCombat()->bFire)
	{
		YuanZuCharacter->GetCombat()->StopFire();

		if (YuanZuCharacter->GetCombat()->EquippedWeapon)
		{
			YuanZuCharacter->GetCombat()->EquippedWeapon->DroppedWeapon();
			YuanZuCharacter->GetCombat()->EquippedWeapon = nullptr;
		}
		
	}

}

void AYuanZuPlayerController::ServerCheckMatchState_Implementation()
{
	AYuanZuGameMode* YZGameMode = Cast<AYuanZuGameMode>(UGameplayStatics::GetGameMode(this));
	if (YZGameMode)
	{
		MatchState = YZGameMode->GetMatchState();
		WarmupTime = YZGameMode->WarmupTime;
		MatchTime = YZGameMode->MatchTime;
		LevelStartingTime = YZGameMode->LevelStartingTime;
		CooldownTime = YZGameMode->CooldownTime;

		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void AYuanZuPlayerController::ClientJoinMidgame_Implementation(FName State, float Warmup, float Match, float LevelStarting, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = LevelStarting;
	MatchState = State;
	CooldownTime = Cooldown;

	OnMatchStateSet(MatchState);
}

void AYuanZuPlayerController::OnRep_MatchState()
{
	OnMatchStateSet(MatchState);
}

void AYuanZuPlayerController::SetHealth(float Health, float NewMaxHealth)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
	bool bHUDVaild = YuanZuHUD &&
		YuanZuHUD->MainUIBase &&
		YuanZuHUD->MainUIBase->CharacterOverlay &&
		YuanZuHUD->MainUIBase->CharacterOverlay->CurrentHealth &&
		YuanZuHUD->MainUIBase->CharacterOverlay->MaxHealth &&
		YuanZuHUD->MainUIBase->CharacterOverlay->HealthProgress;
	if (bHUDVaild)
	{
		const float HealthPercent = Health / NewMaxHealth;
		YuanZuHUD->MainUIBase->CharacterOverlay->HealthProgress->SetPercent(HealthPercent);
		FString CurrentHealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		FString MaxHealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(NewMaxHealth));
		YuanZuHUD->MainUIBase->CharacterOverlay->CurrentHealth->SetText(FText::FromString(CurrentHealthText));
		YuanZuHUD->MainUIBase->CharacterOverlay->MaxHealth->SetText(FText::FromString(MaxHealthText));
	}
}

void AYuanZuPlayerController::SetHUDKillScore(float Score)
{
	//YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
	//bool bHUDVaild =
	//	YuanZuHUD &&
	//	YuanZuHUD->MainUIBase &&
	//	YuanZuHUD->MainUIBase->GameRecord &&
	//	YuanZuHUD->MainUIBase->GameRecord->PlayerRecord &&
	//	YuanZuHUD->MainUIBase->GameRecord->PlayerRecord->KillScoreText;
	//if (bHUDVaild)
	//{
	//	FString KillScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
	//	YuanZuHUD->MainUIBase->GameRecord->PlayerRecord->KillScoreText->SetText(FText::FromString(KillScoreText));
	//}
}

void AYuanZuPlayerController::SetHUDDeathScore(int32 Score)
{

	//YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
	//bool bHUDVaild =
	//	YuanZuHUD &&
	//	YuanZuHUD->MainUIBase &&
	//	YuanZuHUD->MainUIBase->GameRecord &&
	//	YuanZuHUD->MainUIBase->GameRecord->PlayerRecord &&
	//	YuanZuHUD->MainUIBase->GameRecord->PlayerRecord->DeathScoreText;
	//if (bHUDVaild)
	//{
	//	FString DeathScoreText = FString::Printf(TEXT("%d"), Score);
	//	YuanZuHUD->MainUIBase->GameRecord->PlayerRecord->DeathScoreText->SetText(FText::FromString(DeathScoreText));
	//}
}

void AYuanZuPlayerController::SetHUDWeaponAmmo(int32 AmmoNumber)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;

	bool bHUDVaild =
		YuanZuHUD &&
		YuanZuHUD->MainUIBase &&
		YuanZuHUD->MainUIBase->WeaponProperty &&
		YuanZuHUD->MainUIBase->WeaponProperty->CurrentAmmoText;

	if (bHUDVaild)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), AmmoNumber);
		YuanZuHUD->MainUIBase->WeaponProperty->CurrentAmmoText->SetText(FText::FromString(AmmoText));
	}
}

void AYuanZuPlayerController::SetHUDCarriedAmmo(int32 AmmoNumber)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;

	bool bHUDVaild =
		YuanZuHUD &&
		YuanZuHUD->MainUIBase &&
		YuanZuHUD->MainUIBase->WeaponProperty &&
		YuanZuHUD->MainUIBase->WeaponProperty->CarriedAmmoText;
	if (bHUDVaild)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), AmmoNumber);
		YuanZuHUD->MainUIBase->WeaponProperty->CarriedAmmoText->SetText(FText::FromString(AmmoText));
	}
}

void AYuanZuPlayerController::SetHUDGameTime(float GameTime)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
	bool bHUDVaild =
		YuanZuHUD &&
		YuanZuHUD->MainUIBase &&
		YuanZuHUD->MainUIBase->GameTimeText;
	if (bHUDVaild)
	{
		if (GameTime < 0.f)
		{
			YuanZuHUD->MainUIBase->GameTimeText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(GameTime / 60.f);
		int32 Seconds = GameTime - Minutes * 60;

		FString TimeText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		YuanZuHUD->MainUIBase->GameTimeText->SetText(FText::FromString(TimeText));
	}
}

void AYuanZuPlayerController::SetHUDAnnouncementCountdown(float Countdown)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
	bool bHUDVaild =
		YuanZuHUD &&
		YuanZuHUD->Announcement &&
		YuanZuHUD->Announcement->WarupTimeText;
	if (bHUDVaild)
	{
		if (Countdown < 0.f)
		{
			YuanZuHUD->Announcement->WarupTimeText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(Countdown / 60.f);
		int32 Seconds = Countdown - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		YuanZuHUD->Announcement->WarupTimeText->SetText(FText::FromString(CountdownText));
	}
}

void AYuanZuPlayerController::SetHUDPlayerName(FString Name)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;

	bool bHUDVaild =
		YuanZuHUD &&
		YuanZuHUD->MainUIBase &&
		YuanZuHUD->MainUIBase->CharacterOverlay &&
		YuanZuHUD->MainUIBase->CharacterOverlay->PlayerNameText;
	if (bHUDVaild)
	{
		YuanZuHUD->MainUIBase->CharacterOverlay->PlayerNameText->SetText(FText::FromString(Name));
	}
}

void AYuanZuPlayerController::SetHUDPlayerIcon(ETeamType InTeamType)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;

	bool bHUDVaild =
		YuanZuHUD &&
		YuanZuHUD->MainUIBase &&
		YuanZuHUD->MainUIBase->CharacterOverlay &&
		YuanZuHUD->MainUIBase->CharacterOverlay->PlayerIcon;
	if (bHUDVaild)
	{
		YuanZuHUD->MainUIBase->CharacterOverlay->SetPlayerIcon(InTeamType);
	}
}

void AYuanZuPlayerController::SetWeaponProperty(AYuanZuWeapon* Weapon)
{
	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;
	
	bool bHUDVaild =
		YuanZuHUD &&
		YuanZuHUD->MainUIBase &&
		YuanZuHUD->MainUIBase->WeaponProperty;

	if (bHUDVaild)
	{
		YuanZuHUD->MainUIBase->WeaponProperty->SetWeaponNameByWeapon(Weapon);
	}
}

void AYuanZuPlayerController::ShowWeaponProperty(bool bIsVisible)
{
	if (!IsLocalController())return;

	YuanZuHUD = YuanZuHUD == nullptr ? Cast<AYuanZuHUD>(GetHUD()) : YuanZuHUD;

	if (YuanZuHUD && YuanZuHUD->MainUIBase)
	{
		YuanZuHUD->MainUIBase->ShowWeaponProperty(bIsVisible);
	}
}

void AYuanZuPlayerController::OnDestroySessionCompleteForReturnToLogin(bool bWasSuccessful)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMultiplayerSessionsSubsystem* SessionsSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
		{
			SessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(
				this,
				&AYuanZuPlayerController::OnDestroySessionCompleteForReturnToLogin
			);
		}
	}

	// 即使销毁失败，也仍然回登录页，避免卡死在房间里
	UGameplayStatics::OpenLevel(this, *LoginPath);
}

void AYuanZuPlayerController::BeginReturnToLogin()
{
	if (bIsReturningToLogin)
	{
		return;
	}

	bIsReturningToLogin = true;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMultiplayerSessionsSubsystem* SessionsSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
		{
			//防止重复绑定
			SessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &AYuanZuPlayerController::OnDestroySessionCompleteForReturnToLogin);

			SessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &AYuanZuPlayerController::OnDestroySessionCompleteForReturnToLogin);

			SessionsSubsystem->DestroySession();
			return;
		}
	}

	//如果没有子系统，直接回登录图
	UGameplayStatics::OpenLevel(this, *LoginPath);
}

void AYuanZuPlayerController::RequestQuitToLogin()
{
	if (bIsReturningToLogin)
	{
		return;
	}

	if (HasAuthority() && IsLocalController())
	{
		if (AYuanZuGameMode* YGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AYuanZuGameMode>() : nullptr)
		{
			YGameMode->HandleHostQuitToLogin();
			return;
		}
	}

	BeginReturnToLogin();
}

void AYuanZuPlayerController::ClientReturnToLogin_Implementation()
{
	BeginReturnToLogin();
}

void AYuanZuPlayerController::ServerSetPlayerNameFromUI_Implementation(const FString& NewName)
{
	AYuanZuPlayerState* YZPlayerState = GetPlayerState<AYuanZuPlayerState>();
	if (YZPlayerState)
	{
		YZPlayerState->SetPlayerNameCustom(NewName);
	}
}

float AYuanZuPlayerController::GetServerTime()
{
	//客户端 = 0s 服务器 = 2s 客户端 = 2s 服务器 = 4s + 0.5s的延迟
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AYuanZuPlayerController::SetPlayerNameFromUI_Implementation(const FString& NewName)
{
	if (HasAuthority())
	{
		ServerSetPlayerNameFromUI(NewName);
	}
	else
	{
		ServerSetPlayerNameFromUI(NewName);
	}
}

void AYuanZuPlayerController::SetHUDTime()
{
	//剩余时间
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;
	}
	//剩余秒数 = 设置的总时间 - 世界中过去的时间
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDGameTime(TimeLeft);
		}
		if (MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void AYuanZuPlayerController::ServerRequestServerTime_Implementation(float TimeToClientRequest)
{
	//服务器时间
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	//服务器返回时间
	//假设：客户端 = 1；服务器 = 3 客户端传给服务器的TimeToClientRequest = 1
	ClientReportServerTime(TimeToClientRequest, ServerTimeOfReceipt);
}

void AYuanZuPlayerController::ClientReportServerTime_Implementation(float TimeToClientRequest, float TimeServerReceivedClientRequest)
{
	//往返时间 = 1
	//从服务器再到客户端需要1s 此时服务器是 3 + 服务器再到客户端 = 4
	//客户端时间就是2 往返时间：客户端时间 - 客户端传给服务器的TimeToClientRequest = 1
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeToClientRequest;
	//当前服务器时间就是
	//服务器时间4 + 服务器传给客户端的“返”时间 就是（往返时间 * 0.5）= 4.5
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	//客户端和服务器的差值
	//当前服务器时间4.5 - 客户端时间2 = 2.5
	//客户端0秒的时候 服务器2秒 但是客户端想要同步服务器时间 就需要加上“服务器到客户端的延迟 = 0.5” 所以差值就是2.5
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AYuanZuPlayerController::ShowSetting()
{
	if (bIsSettingOpen)
	{
		CloseSetting();
		return;
	}
	else
	{
		if (bIsGameRecordOpen)
		{
			HideGameRecord();
		}
		if (GameSettings == nullptr)
		{
			if (GameSettingsClass)
			{
				GameSettings = CreateWidget<UYuanZuGameSettings>(this, GameSettingsClass);
				if (GameSettings)
				{
					GameSettings->AddToViewport();
				}
			}
		}

		if (GameSettings == nullptr)
		{
			return;
		}

		GameSettings->SetVisibility(ESlateVisibility::Visible);

		//显示鼠标
		bShowMouseCursor = true;

		//禁止角色移动和转镜头
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);

		//输入切到UI，但保留UI未处理输入给PC的机会
		FInputModeGameAndUI InputMode;//输入模式仅用户界面
		InputMode.SetWidgetToFocus(GameSettings->TakeWidget());//将小部件设为焦点状态
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//将鼠标锁定设置为视口模式行为模式为：不将鼠标锁定在视图区域内	
		InputMode.SetHideCursorDuringCapture(false);;//设置输入模式
		SetInputMode(InputMode);

		bIsSettingOpen = true;
	}
}

void AYuanZuPlayerController::CloseSetting()
{
	if (!bIsSettingOpen)
	{
		return;
	}

	if (GameSettings)
	{
		GameSettings->SetVisibility(ESlateVisibility::Hidden);	
	}

	//恢复角色输入
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();

	//输入还给游戏
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	//隐藏鼠标
	bShowMouseCursor = false;

	bIsSettingOpen = false;
}

void AYuanZuPlayerController::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->Move(Value);
	}
}

void AYuanZuPlayerController::Look(const FInputActionValue& Value)
{
	if (YuanZuCharacter)
	{
		YuanZuCharacter->Look(Value);
	}
}

void AYuanZuPlayerController::Jump()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->Jump();

		if (YuanZuCharacter->bIsCrouched)
		{
			YuanZuCharacter->UnCrouch();
		}

		if (YuanZuCharacter->IsSwimming())
		{
			UE_LOG(LogYuanZu, Warning, TEXT("玩家正在游泳，不能跳"));
		}
	}
}

void AYuanZuPlayerController::StopJumping()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->StopJumping();
	}
}

void AYuanZuPlayerController::StartRunning()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->StartRun();
	}
}

void AYuanZuPlayerController::StopRunning()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->StopRun();
	}
}

void AYuanZuPlayerController::EquipButtonPressed()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		if (YuanZuCharacter->GetCombat())
		{
			if (HasAuthority())
			{
				YuanZuCharacter->GetCombat()->EquipWeapon(YuanZuCharacter->GetOverlappingWeapon());
			}
			else
			{
				ServerEquipButtonPressed();
			}
		}
	}
}

void AYuanZuPlayerController::ServerEquipButtonPressed_Implementation()
{
	if (YuanZuCharacter)
	{
		if (YuanZuCharacter->GetCombat())
		{
			YuanZuCharacter->GetCombat()->EquipWeapon(YuanZuCharacter->GetOverlappingWeapon());
		}
	}
}

void AYuanZuPlayerController::Dropped()
{
	if (bDisableGameplay) return;

	if (HasAuthority())
	{
		if (YuanZuCharacter && YuanZuCharacter->GetCombat() && YuanZuCharacter->GetCombat()->EquippedWeapon && !YuanZuCharacter->GetIsAiming())
		{
			YuanZuCharacter->GetCombat()->DroppedWeapon();
		}
	}
	else
	{
		ServerDropped();
	}
}

void AYuanZuPlayerController::ServerDropped_Implementation()
{
	if (YuanZuCharacter && YuanZuCharacter->GetCombat() && YuanZuCharacter->GetCombat()->EquippedWeapon && !YuanZuCharacter->GetIsAiming())
	{
		YuanZuCharacter->GetCombat()->DroppedWeapon();
	}
}

void AYuanZuPlayerController::Reload()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter && YuanZuCharacter->GetCombat() && YuanZuCharacter->GetCombat()->EquippedWeapon)
	{
		YuanZuCharacter->GetCombat()->Reload();
	}
}

void AYuanZuPlayerController::CrouchButtonPressed()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		if (YuanZuCharacter->bIsCrouched)
		{
			YuanZuCharacter->UnCrouch();
		}
		else if (!YuanZuCharacter->GetCharacterMovement()->IsFalling())
		{
			YuanZuCharacter->Crouch();
		}
	}
}

void AYuanZuPlayerController::AimButtonPressed()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter && !YuanZuCharacter->bIsReload && !YuanZuCharacter->IsSwimming())
	{
		if (YuanZuCharacter->GetCombat() && YuanZuCharacter->GetCombat()->EquippedWeapon)
		{
			YuanZuCharacter->GetCombat()->GetAiming(true);
		}
	}
}

void AYuanZuPlayerController::AimButtonReleased()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter && !YuanZuCharacter->IsSwimming())
	{
		if (YuanZuCharacter->GetCombat())
		{
			YuanZuCharacter->GetCombat()->GetAiming(false);
		}
	}
}

void AYuanZuPlayerController::Swim()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		if (!YuanZuCharacter->IsSwimming()) return;

		else
		{
			YuanZuCharacter->StartSwim();
		}
	}
}

void AYuanZuPlayerController::StopSwimming()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		if (!YuanZuCharacter->IsSwimming()) return;
		else
		{
			YuanZuCharacter->StopSwim();
		}
	}
}

void AYuanZuPlayerController::Fire()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		if (!YuanZuCharacter->IsSwimming())
		{
			YuanZuCharacter->FireButtonPressed();
		}
	}
}

void AYuanZuPlayerController::StopFire()
{
	if (bDisableGameplay) return;

	if (YuanZuCharacter)
	{
		YuanZuCharacter->FireButtonReleased();
	}
}

void AYuanZuPlayerController::ShowGameRecord()
{
	if (bIsSettingOpen && bIsGameRecordOpen)return;

	if (YuanZuHUD && YuanZuHUD->MainUIBase)
	{
		bIsGameRecordOpen = true;
		YuanZuHUD->MainUIBase->ShowGameRecord(true);
	}
}

void AYuanZuPlayerController::HideGameRecord()
{
	if (bIsSettingOpen && !bIsGameRecordOpen)return;

	if (YuanZuHUD && YuanZuHUD->MainUIBase)
	{
		bIsGameRecordOpen = false;
		YuanZuHUD->MainUIBase->ShowGameRecord(false);
	}
}

void AYuanZuPlayerController::ClientShowWeaponProperty_Implementation(bool IsVisible)
{
	ShowWeaponProperty(IsVisible);
}
