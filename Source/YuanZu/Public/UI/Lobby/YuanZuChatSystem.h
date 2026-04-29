// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuChatSystem.generated.h"

class UButton;
class UMultiLineEditableTextBox;
class UYuanZuMessage;
class UYuanZuPlayerStateHint;
class UScrollBox;
class AYuanZuLobbyGameState;

struct FYuanZuChatMessageInfo;

UCLASS()
class YUANZU_API UYuanZuChatSystem : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuMessage> MessageClass;

	// 当聊天列表里遇到状态提示消息时，使用这个控件类来创建提示条目。
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuPlayerStateHint> PlayerStateHintClass;

	UPROPERTY(meta = (BindWidget))
	UMultiLineEditableTextBox* PlayerInputTextBox;

	UPROPERTY(meta = (BindWidget))
	UButton* SendButton;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* MessageScrollBox;

	UPROPERTY()
	TObjectPtr<AYuanZuLobbyGameState> CachedLobbyGameState = nullptr;

	FTimerHandle BindLobbyGameStateTimerHandle;

private:
	UFUNCTION()
	void OnClickedSendButton();

	void TryBindLobbyGameState();
	void RefreshMessages(const TArray<FYuanZuChatMessageInfo>& InMessages);
};