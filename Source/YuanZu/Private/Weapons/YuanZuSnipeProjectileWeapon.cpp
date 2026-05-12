// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuSnipeProjectileWeapon.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Character/YuanZuCharacterBase.h"

AYuanZuSnipeProjectileWeapon::AYuanZuSnipeProjectileWeapon()
{

}

void AYuanZuSnipeProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (ScopeClass)
	{
		Scope = CreateWidget<UUserWidget>(GetWorld(), ScopeClass);
	}
}
