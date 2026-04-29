// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuOverHeadWidget.generated.h"

class UTextBlock;

UCLASS()
class YUANZU_API UYuanZuOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayText;

public:
	void SetFDisplayText(FString TextToDispaly);
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);
	
};
