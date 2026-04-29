#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Lobby/Rests/YuanZuPlayerData.h"
#include "YuanZuJoinPlayerMessage.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class YUANZU_API UYuanZuJoinPlayerMessage : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* RemoveButton;

	FString TargetPlayerName = TEXT("");
	int32 TargetPlayerID = INDEX_NONE;

private:
	UFUNCTION()
	void OnClickedRemoveButton();

public:
	void InitJoinPlayerMessage(const FYuanZuPlayerData& InPlayerData);
	void UpdateRemoveButton();
};
