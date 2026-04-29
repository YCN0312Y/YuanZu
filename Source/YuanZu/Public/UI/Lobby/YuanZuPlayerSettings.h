#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YuanZuPlayerSettings.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class YUANZU_API UYuanZuPlayerSettings : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;

private:
	//玩家名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
	//移除玩家
	UPROPERTY(meta = (BindWidget))
	UButton* RemovePlayerButton;
	//禁言
	UPROPERTY(meta = (BindWidget))
	UButton* BannedPostButton;
	//禁言/解除禁言文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BannedPostText;
	//关闭
	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	//当前目标玩家名
	FString TargetPlayerName;
	//当前目标玩家ID
	int32 TargetPlayerId = INDEX_NONE;
	//当前目标玩家是否被禁言
	bool bIsTargetMuted = false;

	//禁言
	UPROPERTY(EditAnywhere, Category = YuanZu)
	FString BannedPost;
	//解除禁言
	UPROPERTY(EditAnywhere, Category = YuanZu)
	FString RemoveBannedPost;


private:
	//刷新“禁言/解除禁言”按钮文字
	void RefreshBannedPostText();
	UFUNCTION()
	void OnClickedRemoveButton();
	UFUNCTION()
	void OnClickedBannedPostButton();
	UFUNCTION()
	void OnClickedCloseButton();

public:
	//初始化右键菜单显示的玩家信息
	void InitRoomPlayerSettings(const FString& InPlayerName, int32 InTargetPlayerId);
	//设置菜单出现在屏幕上的位置
	void SetMenuPosition(const FVector2D& InScreenPosition);
};