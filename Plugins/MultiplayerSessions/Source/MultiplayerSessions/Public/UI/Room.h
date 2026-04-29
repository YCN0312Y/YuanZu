// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Room.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomClicked, URoom*, ClickedRoom);

class UButton;
class UTextBlock;
class UImage;
class FOnlineSessionSearchResult;
class UTexture2D;
class UMenu;

struct FRoomInformation;


UCLASS()
class MULTIPLAYERSESSIONS_API URoom : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	int32 Index = INDEX_NONE;

public:
	//游戏房间
	UPROPERTY(meta = (BindWidget))
	UButton* GameRoomButton;
	//房间名
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomNameText;
	//当前房间人数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentPlayerNumberText;
	//房间最大人数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxPlayerNumberText;
	//网络延迟
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;
	//MS图标
	UPROPERTY(meta = (BindWidget))
	UImage* MSImage;

	FOnRoomClicked OnRoomClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Multiplayer)
	TArray<UTexture2D*> MS;

public:
	//房间初始化
	void Init(const FRoomInformation& Info,int32 NewIndex);
	//设置设置选择的房间
	void SetSelectedState(bool bIsSelected);

private:
	UFUNCTION()
	void OnGameRoomClicked();
	void SetPingImage(int32 PingValue);

public:
	FORCEINLINE int32 GetIndex() const { return Index; }
};
