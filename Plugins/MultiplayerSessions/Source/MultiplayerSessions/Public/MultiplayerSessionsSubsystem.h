#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

class UMenu;
class UPlayerInformation;

extern const FName SESSION_KEY_ROOM_NAME;//房间名
extern const FName SESSION_KEY_MATCH_TYPE;//比赛类型
extern const FName SESSION_KEY_CURRENT_PLAYERS;//当前玩家
extern const FName SESSION_KEY_ROOM_STATE;//房间状态

USTRUCT(BlueprintType)
struct FRoomInformation
{
	GENERATED_BODY()

public:
	//房间名
	FString RoomName;
	//房间当前人数
	int32 RoomCurrentPlayerNumber = 0;
	//房间最大人数
	int32 RoomMaxPlayerNumber = 0;
	//房间ping值
	int32 RoomPing = 0;

};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindRoomsComplete, const TArray<FRoomInformation>&);

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();

public:
	//应该创建子系统
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//初始化
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//反初始化
	virtual void Deinitialize() override;

public:
	//创建多人会话完成
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	//查询到的房间
	TArray<FRoomInformation> FindRoom;
	//查询房间代理
	FOnFindRoomsComplete OnFindRoomsComplete;
	//上一个搜索的结果
	TArray<FOnlineSessionSearchResult> LastSearchResults;

	//玩家名称
	FString CachedPlayerName;
	//缓存的当前房间内玩家数量
	int32 CachedPlayerCount = 2;

private:
	//会话接口
	IOnlineSessionPtr SessionInterface;
	//创建会话的设置
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	//搜索到的会话
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//在线会话代理----------
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	//列表房间名
	FString LastRoomName;
	//创建会话时销毁
	bool bCreateSessionOnDestroy = false;
	//最大人数
	int32 LastNumPublicConnections = 4;
	//比赛类型
	FString LastMatchType;
	//会话名称
	FName NewSessionName = NAME_GameSession;

private:
	//刷新会话接口
	void RefreshSessionInterface();
	//获取当前子系统名称
	FName GetCurrentSubsystemName() const;
	//是否使用Steam
	bool IsUsingSteam();

	//为会话编码房间名称
	static FString EncodeRoomNameForSession(const FString& InName);
	//从会话中解码房间名称
	static FString DecodeRoomNameFromSession(const FString& InEncoded);
public:
	//会话菜单调用方法----------
	//创建会话
	void CreateSession(const FString& RoomName, int32 NumPublicConnections, FString MathType);
	//查询会话
	void FindSession(int32 MaxSearchResults);
	//加入会话
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	//销毁会话
	void DestroySession();
	//开始会话
	void StartSession();

	//更新当前玩家数量
	void UpdateCurrentPlayerCount(int32 NewCount);
	//设置缓存玩家名称
	void SetCachedPlayerName(const FString& NewName);
	//设置缓存创建房间时设置的最大可以加入的玩家数量
	void SetCachedPlayerCount(const int32& NewCount);
	//游戏开始就锁房间
	void LockRoom();

	//获取缓存玩家名称
	FString GetCachedPlayerName() const { return CachedPlayerName; }
	//获取缓存创建房间时设置的最大可以加入的玩家数量
	int32 GetCachedPlayerCount() const {return CachedPlayerCount;}

protected:
	//执行函数的参数列表是代理的参数
	void OnCreateSessionCompleteDelegate(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsCompleteDelegate(bool bWasSuccessful);
	void OnJoinSessionCompleteDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionCompleteDelegate(FName SessionName, bool bWasSuccessful);
	void OnStartSessionCompleteDelegate(FName SessionName, bool bWasSuccessful);


};
