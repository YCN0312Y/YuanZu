// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/YuanZuOverHeadWidget.h"
#include "Components/TextBlock.h"

void UYuanZuOverHeadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}

void UYuanZuOverHeadWidget::SetFDisplayText(FString TextToDispaly)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(*TextToDispaly));
	}
}

void UYuanZuOverHeadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;

	switch (RemoteRole)
	{
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	}
	FString RemoteRoleStr = FString::Printf(TEXT("Remote Role(%s)"), *Role);
	SetFDisplayText(RemoteRoleStr);

}
