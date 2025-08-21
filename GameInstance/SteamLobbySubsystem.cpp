#include "SteamLobbySubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"

static const FName SETTING_MAPNAME(TEXT("MAPNAME"));
static const FName SETTING_GAMEMODE(TEXT("GAMEMODE"));

void USteamLobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();

        if (SessionInterface.IsValid())
        {
            // Bind delegates
            CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
                FOnCreateSessionCompleteDelegate::CreateUObject(this, &USteamLobbySubsystem::OnCreateSessionComplete));

            JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
                FOnJoinSessionCompleteDelegate::CreateUObject(this, &USteamLobbySubsystem::OnJoinSessionComplete));

            FindSessionsCompleteHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
                FOnFindSessionsCompleteDelegate::CreateUObject(this, &USteamLobbySubsystem::OnFindSessionsComplete));

            DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
                FOnDestroySessionCompleteDelegate::CreateUObject(this, &USteamLobbySubsystem::OnDestroySessionComplete));

            UE_LOG(LogTemp, Log, TEXT("SteamLobbySubsystem initialized"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No Online Subsystem found"));
    }

    bIsInLobby = false;
}

void USteamLobbySubsystem::Deinitialize()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
    }

    Super::Deinitialize();
}

void USteamLobbySubsystem::CreateLobby(const FLobbySettings& Settings)
{
    if (!SessionInterface.IsValid())
    {
        OnLobbyCreated.Broadcast(false);
        return;
    }

    CurrentLobbySettings = Settings;

    TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->bIsLANMatch = false;
    SessionSettings->bUsesPresence = true;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bAllowInvites = true;
    SessionSettings->bShouldAdvertise = !Settings.bIsPrivate;
    SessionSettings->NumPublicConnections = Settings.MaxPlayers;
    SessionSettings->NumPrivateConnections = 0;
    SessionSettings->bUseLobbiesIfAvailable = true;

    // Custom settings
    SessionSettings->Set(SETTING_MAPNAME, Settings.MapName, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->Set(SETTING_GAMEMODE, Settings.GameMode, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->Set(TEXT("LobbyMap"), Settings.LobbyName, EOnlineDataAdvertisementType::ViaOnlineService);

    if (Settings.bIsPrivate && !Settings.Password.IsEmpty())
    {
        SessionSettings->Set(TEXT("Password"), Settings.Password, EOnlineDataAdvertisementType::ViaOnlineService);
    }

    SessionInterface->CreateSession(0, TEXT("LobbyMap"), *SessionSettings);
}

void USteamLobbySubsystem::JoinLobby(const FString& LobbyID, const FString& Password)
{
    if (!SessionInterface.IsValid())
    {
        OnLobbyJoined.Broadcast(false);
        return;
    }

    // Implementation depends on how you store lobby references
    // This is a simplified version
    UE_LOG(LogTemp, Log, TEXT("Attempting to join lobby: %s"), *LobbyID);
}

void USteamLobbySubsystem::LeaveLobby()
{
    if (!SessionInterface.IsValid() || !bIsInLobby) return;

    SessionInterface->DestroySession(TEXT("LobbyMap"));
}

void USteamLobbySubsystem::SearchLobbies()
{
    if (!SessionInterface.IsValid())
    {
        OnLobbySearchComplete.Broadcast(false, TArray<FString>());
        return;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = 50;
    SessionSearch->PingBucketSize = 50;

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void USteamLobbySubsystem::InvitePlayerToLobby(const FString& FriendSteamID)
{
    if (!SessionInterface.IsValid() || !bIsInLobby) return;

    // Steam invitation logic would go here
    UE_LOG(LogTemp, Log, TEXT("Inviting player %s to lobby"), *FriendSteamID);
}

// Delegate Handlers
void USteamLobbySubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        bIsInLobby = true;
        CurrentLobbyID = SessionName.ToString();

        // Travel to lobby level
        UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Maps/LobbyMap"), true, TEXT("listen"));
    }

    OnLobbyCreated.Broadcast(bWasSuccessful);
    UE_LOG(LogTemp, Log, TEXT("Lobby creation %s"), bWasSuccessful ? TEXT("successful") : TEXT("failed"));
}

void USteamLobbySubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);

    if (bSuccess)
    {
        bIsInLobby = true;
        CurrentLobbyID = SessionName.ToString();

        // Get connection string and travel
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
        }
    }

    OnLobbyJoined.Broadcast(bSuccess);
}

void USteamLobbySubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    TArray<FString> LobbyList;

    if (bWasSuccessful && SessionSearch.IsValid())
    {
        for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
        {
            FString LobbyName;
            Result.Session.SessionSettings.Get(TEXT("LobbyName"), LobbyName);
            LobbyList.Add(LobbyName);
        }
    }

    OnLobbySearchComplete.Broadcast(bWasSuccessful, LobbyList);
}

void USteamLobbySubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    bIsInLobby = false;
    CurrentLobbyID.Empty();
    UE_LOG(LogTemp, Log, TEXT("Left lobby"));
}