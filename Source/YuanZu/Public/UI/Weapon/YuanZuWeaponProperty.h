// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuWeaponProperty.generated.h"

class UTextBlock;
class AYuanZuWeapon;
class UTexture2D;
class UImage;
class AYuanZuWeapon;

struct FYuanZuWeaponData;

UCLASS()
class YUANZU_API UYuanZuWeaponProperty : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	
public:
	//携带的弹药数量
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoText;
	//当前武器的弹药数量
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentAmmoText;
	//枪械名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponNameText;
	//枪械图标
	UPROPERTY(meta = (BindWidget))
	UImage * WeaponImage;
	//枪械图标
	UPROPERTY(EditAnywhere, Category = YuanZu, meta = (AllowPrivateAccess = "true"))
	TArray<FYuanZuWeaponData>WeaponIcon;

	UPROPERTY()
	AYuanZuWeapon* Weapon;

public:
	//设置武器名称
	UFUNCTION(BlueprintCallable, Category = YuanZu)
	void SetWeaponName(const FText& InWeaponName);
	//设置对应武器名称
	UFUNCTION(BlueprintCallable, Category = YuanZu)
	void SetWeaponNameByWeapon(AYuanZuWeapon* InWeapon);
};
