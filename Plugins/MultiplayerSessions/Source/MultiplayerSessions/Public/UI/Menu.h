// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Blueprint/UserWidget.h"
#include "Menu.generated.h"

class UButton;
class UTextBlock;
class UCheckBox;
class UWidgetSwitcher;
class UEditableTextBox;
class UBorder;
class UMultiplayerSessionsSubsystem;
class UWidgetAnimation;
class UPlayerInformation;
class UCircularThrobber;
class USizeBox;
class URoom;
class UScrollBox;
class UOverlay;

struct FTimerHandle;
struct FRoomInformation;

UENUM(BlueprintType)
enum class EButtonState : uint8
{
	EBS_Pressed,
	EBS_Hovered,
	EBS_Unhovered,
	EBS_Released,
	EBS_Disabled,

	EBS_Max

};

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	//初始化
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	//创建房间
	UPROPERTY(meta = (BindWidget))
	UButton* CreateButton;
	//加入房间
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;
	//创建房间文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CreateText;
	//加入房间文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* JoinText;
	//创建房间高亮
	UPROPERTY(meta = (BindWidget))
	UBorder* CreateHeightLight;
	//加入房间高亮
	UPROPERTY(meta = (BindWidget))
	UBorder* JoinHeightLight;
	//房间名过长提示动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* RoomHintAnimation;
	//创建房间时的房间名称
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* CreateRoomNameText;
	//房间名过长提示文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomHintText;
	//添加人数
	UPROPERTY(meta = (BindWidget))
	UButton* AddPeople;
	//减少人数
	UPROPERTY(meta = (BindWidget))
	UButton* SubPeople;
	//玩家数量提示
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNumberText;
	//玩家数量提示动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* PlayerNumberHintAnimation;
	//搜索游戏
	UPROPERTY(meta = (BindWidget))
	UButton* SearchGameButton;
	//搜索游戏文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SearchGameText;
	//搜索中等待
	UPROPERTY(meta = (BindWidget))
	UCircularThrobber* SearchCircularThrobber;
	//游戏房间列表
	UPROPERTY(meta = (BindWidget))
	UScrollBox* GameRoomPlan;
	//创建房间和加入房间的切换
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* CreateAndJoin;
	//创建游戏
	UPROPERTY(meta = (BindWidget))
	UButton* CreateGameButton;
	//加入游戏文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* JoinGameText;
	//等待控件
	UPROPERTY(meta = (BindWidget))
	UUserWidget* LoadingWidget;
	//阻挡玩家输入
	UPROPERTY(meta = (BindWidget))
	UOverlay* BlockPlayerOverlay;

	//搜索到的房间
	URoom* Room;

	//玩家数量
	int32 PlayerNumber = 2;
	//子系统指针
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	//匹配类型
	FString MatchType{ TEXT("FreeForAll") };
	//等待区的地图
	FString PathToHall{ ("") };

	//房间类
	UPROPERTY(EditAnywhere, Category = "YuanZu|Room")
	TSubclassOf<URoom>RoomClass;

	//选中的房间
	int32 SelectedRoomIndex = INDEX_NONE;
	//上一个选择的房间
	UPROPERTY()
	URoom* LastSelectedRoom = nullptr;

public:
	//返回
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;
	//返回文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReturnText;
	//返回高亮
	UPROPERTY(meta = (BindWidget))
	UBorder* ReturnHeightLight;
	//输入房间人数
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InputPeopleNumber;
	//加入游戏
	UPROPERTY(meta = (BindWidget))
	UButton* JoinGameButton;

	FString CreateRoomName;

public:
	//菜单设置
	UFUNCTION(BlueprintCallable)
	void MenuSetup(FString TypeToMatch = FString(TEXT("FreeForAll")), FString HallPath = FString(TEXT("/Game/ThirdPerson/Maps/Hall")));

	//使用枚举设置按键样式
	void ButtonStyle(UBorder* InputBorder, UButton* InputButton, UTextBlock* InputText, EButtonState NewButtonState);
	//设置按键样式
	void ButtonSize(UButton* InputButton, UTextBlock* InputText, float Size, FVector2D FontSize);

private:
	//按键禁用
	void ButtonEnabled();

	/*
	* 回调函数
	*/
	//创建房间
	UFUNCTION()
	void OnHoveredCreate() { ButtonStyle(CreateHeightLight, CreateButton, CreateText, EButtonState::EBS_Hovered); }
	UFUNCTION()
	void OnUnhoveredCreate() { ButtonStyle(CreateHeightLight, CreateButton, CreateText, EButtonState::EBS_Unhovered); }

	//加入房间
	UFUNCTION()
	void OnClickedJoin();
	UFUNCTION()
	void OnHoveredJoin() { ButtonStyle(JoinHeightLight, JoinButton, JoinText, EButtonState::EBS_Hovered); }
	UFUNCTION()
	void OnUnhoveredJoin() { ButtonStyle(JoinHeightLight, JoinButton, JoinText, EButtonState::EBS_Unhovered); }

	//加入游戏
	UFUNCTION()
	void OnHoveredJoinGame() { ButtonStyle(nullptr, JoinGameButton, JoinGameText, EButtonState::EBS_Hovered); }
	UFUNCTION()
	void OnUnhoveredJoinGame() { ButtonStyle(nullptr, JoinGameButton, JoinGameText, EButtonState::EBS_Unhovered); }
	UFUNCTION()
	void OnReleasedJoinGame() { ButtonStyle(nullptr, JoinGameButton, JoinGameText, EButtonState::EBS_Released); }

	//添加人数
	UFUNCTION()
	void OnAddPeople();
	//减少人数
	UFUNCTION()
	void OnSubPeople();
	//输入文字文本更新时
	UFUNCTION()
	void OnInputPeopleNumberChanged(const FText& Text);
	//输入文字文本提交时
	UFUNCTION()
	void OnInputPeopleNumberCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	//搜索游戏
	UFUNCTION()
	void OnClickedSearchGame();
	UFUNCTION()
	void OnFindRoom(const TArray<FRoomInformation>& Rooms);
	UFUNCTION()
	void HandleRoomSelected(URoom* ClickedRoom);
	//创建的房间名称更新
	UFUNCTION()
	void OnCreateRoomNameTextChanged(const FText& Text);
	//创建的房间名称提交
	UFUNCTION()
	void OnCreateRoomNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	//创建游戏
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void CreateButtonClicked();
	//加入游戏
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void JoinButtonClicked();
	//退出 菜单销毁
	void MenuTearDown();

protected:
	//创建会话
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	//查询会话
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);
	//加入会话
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

public:
	UFUNCTION()
	void OnClickedCreate();

	void OnReleasedJoin();

	FORCEINLINE UButton* GetCreateButton()const { return CreateButton; }
	FORCEINLINE UBorder* GetJoinHeightLight()const { return JoinHeightLight; }

};
