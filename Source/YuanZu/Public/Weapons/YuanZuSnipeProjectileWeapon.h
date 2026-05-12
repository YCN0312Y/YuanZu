// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/YuanZuProjectileWeapon.h"
#include "YuanZuSnipeProjectileWeapon.generated.h"

class UCameraComponent;
class UUserWidget;

UCLASS()
class YUANZU_API AYuanZuSnipeProjectileWeapon : public AYuanZuProjectileWeapon
{
	GENERATED_BODY()
public:
	AYuanZuSnipeProjectileWeapon();

protected:
	virtual void BeginPlay()override;

public:
	//Ãé×¼¾µÀà
	UPROPERTY(EditAnywhere, Category = "YuanZu|Aim")
	TSubclassOf<UUserWidget>ScopeClass;
	//Ãé×¼¾µ
	UPROPERTY(BlueprintReadWrite, Category = "YuanZu|Aim")
	TObjectPtr<UUserWidget>Scope;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "YuanZu|Weapon")
	void OpenScope(bool bIsAim);

};
