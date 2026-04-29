// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuPickUp.generated.h"

class UTextBlock;
class AYuanZuWeapon;

UCLASS()
class YUANZU_API UYuanZuPickUp : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct()override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemsText;

public:
	UPROPERTY()
	AYuanZuWeapon* Weapon;

private:
	UFUNCTION()
	FText GetItemText();

public:
	void SetWeapon(AYuanZuWeapon* InWeapon);
};
