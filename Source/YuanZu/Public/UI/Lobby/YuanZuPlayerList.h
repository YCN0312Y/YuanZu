#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuPlayerList.generated.h"

class UScrollBox;
class UYuanZuJoinPlayerMessage;

UCLASS()
class YUANZU_API UYuanZuPlayerList : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerListScrollBox;

	UPROPERTY()
	UYuanZuJoinPlayerMessage* JoinPlayerMessage;

	UPROPERTY(EditAnywhere, Category = YuanZu)
	TSubclassOf<UYuanZuJoinPlayerMessage> JoinPlayerMessageClass;

private:
	void RefreshPlayerList();
};
