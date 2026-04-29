#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "UI/PlayerInformation.h"
#include "UI/Menu.h"
#include "MultiplayerSessions/Log/YUYI_MlutiplayerLog.h"

const FName SESSION_KEY_ROOM_NAME(TEXT("ROOM_NAME"));
const FName SESSION_KEY_MATCH_TYPE(TEXT("Match_Type"));
const FName SESSION_KEY_CURRENT_PLAYERS(TEXT("CURRENT_PLAYERS"));
const FName SESSION_KEY_ROOM_STATE(TEXT("ROOM_STATE"));

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionCompleteDelegate)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsCompleteDelegate)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionCompleteDelegate)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionCompleteDelegate)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnStartSessionCompleteDelegate))
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
    }
}

bool UMultiplayerSessionsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    Super::ShouldCreateSubsystem(Outer);

    return true;
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LastSessionSettings.Reset();
    LastSessionSearch.Reset();

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
    }
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    SessionInterface.Reset();
    LastSessionSettings.Reset();
    LastSessionSearch.Reset();
}

//刷新会话接口
void UMultiplayerSessionsSubsystem::RefreshSessionInterface()
{
    //获取在线子系统名称
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
        UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("OSS = [%s]"), *Subsystem->GetSubsystemName().ToString());
    }
    else
    {
        SessionInterface.Reset();
        UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("No OnlineSubsystem"));
    }
}

FName UMultiplayerSessionsSubsystem::GetCurrentSubsystemName() const
{
    if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        return Subsystem->GetSubsystemName();
    }
    return NAME_None;
}

bool UMultiplayerSessionsSubsystem::IsUsingSteam()
{
    return GetCurrentSubsystemName() == FName(TEXT("STEAM"));
}

void UMultiplayerSessionsSubsystem::LockRoom()
{
    RefreshSessionInterface();

    if (!SessionInterface.IsValid())return;

    //会话名
    FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (!NamedSession) return;

    FOnlineSessionSettings NewSessionSettings = NamedSession->SessionSettings;

    NewSessionSettings.bShouldAdvertise = false;//不允许广播推送
    NewSessionSettings.bAllowJoinInProgress = false;//比赛进行中不允许有玩家加入
    NewSessionSettings.bAllowJoinViaPresence = false;//不允许玩家在线状态加入
    NewSessionSettings.bUsesPresence = true;//用户在线状态信息
    NewSessionSettings.bUseLobbiesIfAvailable = IsUsingSteam();
    NewSessionSettings.Set(SESSION_KEY_ROOM_STATE, FString(TEXT("InGame")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    //设置一个新的房间设置然后再给房间设置
    SessionInterface->UpdateSession(NAME_GameSession, NewSessionSettings, true);
}

void UMultiplayerSessionsSubsystem::UpdateCurrentPlayerCount(int32 NewCount)
{
    RefreshSessionInterface();

    if (!SessionInterface.IsValid()) return;

    FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (!NamedSession) return;

    FOnlineSessionSettings NewSettings = NamedSession->SessionSettings;
    NewSettings.Set(SESSION_KEY_CURRENT_PLAYERS, NewCount, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    SessionInterface->UpdateSession(NAME_GameSession, NewSettings, true);

    UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("UpdateCurrentPlayerCount = %d"), NewCount);
}

FString UMultiplayerSessionsSubsystem::EncodeRoomNameForSession(const FString& InName)
{
    //将输入的房间名变成uft8类型
    FTCHARToUTF8 Convert(*InName);
    const uint8* Data = reinterpret_cast<const uint8*>(Convert.Get());
    return FBase64::Encode(Data, Convert.Length());
}

FString UMultiplayerSessionsSubsystem::DecodeRoomNameFromSession(const FString& InEncoded)
{
    TArray<uint8> DecodedBytes;
    if (!FBase64::Decode(InEncoded, DecodedBytes))
    {
        //不是Base64，说明可能是旧数据或英文，直接返回原值
        return InEncoded;
    }

    DecodedBytes.Add(0);
    return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(DecodedBytes.GetData())));
}

//创建会话
void UMultiplayerSessionsSubsystem::CreateSession(const FString& RoomName, int32 NumPublicConnections, FString MathType)
{
    RefreshSessionInterface();

    if (!SessionInterface.IsValid())
    {
        UE_LOG(YUYI_MlutiplayerLog, Error, TEXT("创建会话失败：会话接口无效！"));
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }
    const FString FinalRoomName = RoomName.TrimStartAndEnd();
    if (FinalRoomName.IsEmpty())
    {
        UE_LOG(YUYI_MlutiplayerLog, Error, TEXT("创建会话失败：房间名称为空！"));
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }
    //检查当前会话是否存在同名 如果同名就销毁
    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        bCreateSessionOnDestroy = true;
        LastNumPublicConnections = NumPublicConnections;
        LastMatchType = MathType;
        LastRoomName = FinalRoomName;
        DestroySession();
        return;
    }
    //将委托存储在CreateSessionCompleteDelegateHandle句柄中
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());

    const bool bUsingSteam = IsUsingSteam();

    LastSessionSettings->bIsLANMatch = false;//是否使用局域网true则使用局域网false则使用steam
    LastSessionSettings->NumPublicConnections = NumPublicConnections;//设置玩家数量
    LastSessionSettings->bAllowJoinInProgress = true;//比赛进行中允许有玩家加入
    LastSessionSettings->bAllowJoinViaPresence = true;//允许玩家在线状态加入
    LastSessionSettings->bShouldAdvertise = true;//允许广播推送
    LastSessionSettings->bUsesPresence = true;//用户在线状态信息
    LastSessionSettings->bUseLobbiesIfAvailable = bUsingSteam;//如果平台支持的话，优先使用插件式API

    if (bUsingSteam)
    {
        UE_LOG(LogTemp, Warning, TEXT("使用Steam游戏大厅设置创建会话"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("使用存在状态设置创建会话"));
    }

    const FString EncodeRoomName = EncodeRoomNameForSession(FinalRoomName);

    UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("创建房间，房间名为：[%s]"), *FinalRoomName);

    LastSessionSettings->Set(SESSION_KEY_MATCH_TYPE, MathType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);//通过在线服务和ping宣传
    LastSessionSettings->Set(SESSION_KEY_ROOM_NAME, EncodeRoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->Set(SESSION_KEY_CURRENT_PLAYERS, 1, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->Set(SESSION_KEY_ROOM_STATE, FString(TEXT("Lobby")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->BuildUniqueId = GetBuildUniqueId();

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();//从控制器获取第一个本地玩家
    if (!LocalPlayer)
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        MultiplayerOnCreateSessionComplete.Broadcast(false);
        return;
    }
    //LocalPlayer->GetPreferredUniqueNetId()是创建房间玩家的ID
    //NAME_GameSession是会话名称
    //LastSessionSettings是新会话设置
    if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NewSessionName, *LastSessionSettings))
    {
        //创建失败就清除委托
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        //广播自定义委托
        MultiplayerOnCreateSessionComplete.Broadcast(false);
    }

}
//查询会话
void UMultiplayerSessionsSubsystem::FindSession(int32 MaxSearchResults)
{
    RefreshSessionInterface();

    if (!SessionInterface.IsValid())
    {
        UE_LOG(YUYI_MlutiplayerLog, Error, TEXT("搜索会话失败：会话接口无效！"));
        MultiplayerOnFindSessionsComplete.Broadcast({}, false);
        return;
    }

    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());//搜索结果

    LastSessionSearch->MaxSearchResults = MaxSearchResults;//最大搜索结果
    LastSessionSearch->bIsLanQuery = false;//是否使用局域网true则使用局域网false则使用steam
    LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();//从控制器获取第一个本地玩家
    if (!LocalPlayer)
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
        MultiplayerOnFindSessionsComplete.Broadcast({}, false);
        return;
    }

    if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

        MultiplayerOnFindSessionsComplete.Broadcast({}, false);
    }
}

//加入会话
void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();//从控制器获取第一个本地玩家
    if (!LocalPlayer)
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }
    if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    }
}

//销毁会话
void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }

    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
    if (!SessionInterface->DestroySession(NAME_GameSession))
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        MultiplayerOnDestroySessionComplete.Broadcast(false);
    }

}

//开始会话
void UMultiplayerSessionsSubsystem::StartSession()
{
    RefreshSessionInterface();

    if (!SessionInterface.IsValid())
    {
        MultiplayerOnStartSessionComplete.Broadcast(false);
        return;
    }

    StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

    if (!SessionInterface->StartSession(NewSessionName))
    {
        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
        MultiplayerOnStartSessionComplete.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionCompleteDelegate(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }

    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsCompleteDelegate(bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }

    //先清空 避免搜索失败时还显示上一次的数据
    FindRoom.Empty();
    LastSearchResults.Empty();

    if (!bWasSuccessful || !LastSessionSearch.IsValid())
    {
        MultiplayerOnFindSessionsComplete.Broadcast({}, false);
        OnFindRoomsComplete.Broadcast(FindRoom);
        return;
    }
        
    TArray<FOnlineSessionSearchResult> FilteredSearchResults;

    //房间列表搜索到的结果
    for (const FOnlineSessionSearchResult& Result : LastSessionSearch->SearchResults)
    {
        FRoomInformation RoomInfo;
        //房间状态
        FString RoomState;
        Result.Session.SessionSettings.Get(SESSION_KEY_ROOM_STATE, RoomState);
        if (RoomState != TEXT("Lobby"))
        {
            continue;
        }

        FString EncodedName;
        Result.Session.SessionSettings.Get(SESSION_KEY_ROOM_NAME, EncodedName);
        EncodedName = EncodedName.TrimStartAndEnd();

        UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("FindSession EncodedName = [%s]"), *EncodedName);

        if (EncodedName.IsEmpty())
        {
            continue;
        }

        const FString DecodedName = DecodeRoomNameFromSession(EncodedName);

        UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("FindSession DecodedName = [%s]"), *DecodedName);

        RoomInfo.RoomName = DecodedName + TEXT("的房间");

        //最大可加入的玩家
        const int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
        //当前还有房间还有多少空位
        const int32 OpenPlayers = Result.Session.NumOpenPublicConnections;

        //当前人数
        int32 CurrentPlayers = MaxPlayers - OpenPlayers;

        //防止人数越界
        CurrentPlayers = FMath::Clamp(CurrentPlayers, 1, MaxPlayers);

        RoomInfo.RoomMaxPlayerNumber = MaxPlayers;
        RoomInfo.RoomCurrentPlayerNumber = CurrentPlayers;

        UE_LOG(YUYI_MlutiplayerLog, Warning, TEXT("Room[%s] MaxPlayers=%d OpenPlayers=%d CurrentPlayers=%d"), *RoomInfo.RoomName, MaxPlayers, OpenPlayers, CurrentPlayers);

        if (Result.PingInMs == 9999)
        {
            RoomInfo.RoomPing = FMath::RandRange(10, 300);
        }
        else
        {
            RoomInfo.RoomPing = Result.PingInMs;
        }

        FindRoom.Add(RoomInfo);
        FilteredSearchResults.Add(Result);
    }

    LastSearchResults = MoveTemp(FilteredSearchResults);

    MultiplayerOnFindSessionsComplete.Broadcast(LastSearchResults, true);
    OnFindRoomsComplete.Broadcast(FindRoom);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionCompleteDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        MultiplayerOnJoinSessionComplete.Broadcast(Result);
        return;
    }

    FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

    MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionCompleteDelegate(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    }

    if (bWasSuccessful && bCreateSessionOnDestroy)
    {
        bCreateSessionOnDestroy = false;
        CreateSession(LastRoomName, LastNumPublicConnections, LastMatchType);
        return;
    }

    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionCompleteDelegate(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface)
    {
        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
    }

    MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::SetCachedPlayerName(const FString& NewName)
{
    CachedPlayerName = NewName;
}

void UMultiplayerSessionsSubsystem::SetCachedPlayerCount(const int32& NewCount)
{
    CachedPlayerCount = NewCount;
}
