// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/YuanZuPlayerState.h"
#include "Character/YuanZuCharacterBase.h"
#include "Gameplay/YuanZuPlayerController.h"
#include "Net/UnrealNetwork.h"

void AYuanZuPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYuanZuPlayerState, DeathScore);
	DOREPLIFETIME(AYuanZuPlayerState, PlayerNameCustom);
	DOREPLIFETIME(AYuanZuPlayerState, TeamType);
}

void AYuanZuPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	YuanZuCharacter = YuanZuCharacter == nullptr ? Cast<AYuanZuCharacterBase>(GetPawn()) : YuanZuCharacter;
	if (YuanZuCharacter)
	{
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDDeathScore(DeathScore);

		}
	}

}

void AYuanZuPlayerState::AddToKillScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	YuanZuCharacter = YuanZuCharacter == nullptr ? Cast<AYuanZuCharacterBase>(GetPawn()) : YuanZuCharacter;
	if (YuanZuCharacter)
	{
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDKillScore(GetScore());

		}
	}

}

void AYuanZuPlayerState::AddToDeathScore(int32 DeathAmount)
{
	DeathScore += DeathAmount;

	YuanZuCharacter = YuanZuCharacter == nullptr ? Cast<AYuanZuCharacterBase>(GetPawn()) : YuanZuCharacter;
	if (YuanZuCharacter)
	{
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDDeathScore(DeathScore);

		}
	}

}

void AYuanZuPlayerState::OnRep_DeathScore()
{
	YuanZuCharacter = YuanZuCharacter == nullptr ? Cast<AYuanZuCharacterBase>(GetPawn()) : YuanZuCharacter;
	if (YuanZuCharacter)
	{
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDDeathScore(DeathScore);

		}
	}

}

void AYuanZuPlayerState::SetPlayerNameCustom(const FString& NewName)
{
	PlayerNameCustom = NewName;
	UE_LOG(LogTemp, Warning, TEXT("%s"), *NewName);
	YuanZuCharacter = YuanZuCharacter == nullptr ? Cast<AYuanZuCharacterBase>(GetPawn()) : YuanZuCharacter;
	if (YuanZuCharacter)
	{
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDPlayerName(PlayerNameCustom);

		}
	}
}

void AYuanZuPlayerState::SetPlayerTeam(ETeamType NewTeamType)
{
	TeamType = NewTeamType;

	AYuanZuCharacterBase* YuanZuC = Cast<AYuanZuCharacterBase>(GetPawn());
	if (YuanZuC)
	{
		YuanZuC->SetCharacterMaterial(NewTeamType);
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuC->Controller) : YuanZuPlayerController;

		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDPlayerIcon(NewTeamType);
		}
	}
}

void AYuanZuPlayerState::OnRep_PlayerNameCustom()
{
	YuanZuCharacter = YuanZuCharacter == nullptr ? Cast<AYuanZuCharacterBase>(GetPawn()) : YuanZuCharacter;
	if (YuanZuCharacter)
	{
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuCharacter->Controller) : YuanZuPlayerController;
		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDPlayerName(PlayerNameCustom);
		}
	}
}

void AYuanZuPlayerState::OnRep_TeamType()
{
	AYuanZuCharacterBase* YuanZuC = Cast<AYuanZuCharacterBase>(GetPawn());
	if (YuanZuC)
	{
		YuanZuC->SetCharacterMaterial(TeamType);
		YuanZuPlayerController = YuanZuPlayerController == nullptr ? Cast<AYuanZuPlayerController>(YuanZuC->Controller) : YuanZuPlayerController;

		if (YuanZuPlayerController)
		{
			YuanZuPlayerController->SetHUDPlayerIcon(TeamType);
		}
	}
}
