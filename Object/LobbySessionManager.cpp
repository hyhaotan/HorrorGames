#include "LobbySessionManager.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"

const FName ULobbySessionManager::SESSION_NAME = NAME_GameSession;
const FName ULobbySessionManager::SESSION_GAME_TYPE = TEXT("LobbySession");

ULobbySessionManager::ULobbySessionManager()
{
    InitializeSessionInterface();
}

void ULobbySessionManager::InitializeSessionInterface()
{
    // Get any available online subsystem
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();

        FName SubsystemName = OnlineSubsystem->GetSubsystemName();
        UE_LOG(LogTemp, Log, TEXT("SessionManager initialized with: %s"), *SubsystemName.ToString());

        if (SessionInterface.IsValid())
        {
            // Only bind delegates if we have a valid session interface
            CreateSessionDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
                FOnCreateSessionCompleteDelegate::CreateUObject(this, &ULobbySessionManager::OnCreateSessionComplete));

            DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
                FOnDestroySessionCompleteDelegate::CreateUObject(this, &ULobbySessionManager::OnDestroySessionComplete));
        }

        // Check if this is NULL subsystem (offline mode)
        if (SubsystemName == TEXT("NULL"))
        {
            UE_LOG(LogTemp, Warning, TEXT("Running in offline mode - limited multiplayer functionality"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No Online Subsystem found!"));
    }
}

void ULobbySessionManager::CreateLobbySession(int32 MaxPlayers)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("No session interface - loading lobby directly"));
        // Fallback: directly load lobby map for offline testing
        UGameplayStatics::OpenLevel(GetWorld(), TEXT("LobbyMap"), true, TEXT("listen"));
        OnSessionCreated.Broadcast(true);
        return;
    }

    // Check if we're using NULL subsystem (offline)
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS && OSS->GetSubsystemName() == TEXT("NULL"))
    {
        UE_LOG(LogTemp, Log, TEXT("Creating offline session"));
        // For NULL subsystem, just load the level
        UGameplayStatics::OpenLevel(GetWorld(), TEXT("LobbyMap"), true, TEXT("listen"));
        OnSessionCreated.Broadcast(true);
        return;
    }

    // Destroy existing session first
    FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
    if (ExistingSession)
    {
        SessionInterface->DestroySession(SESSION_NAME);
        return;
    }

    // Create session settings
    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = true; // Use LAN for testing
    SessionSettings.bUsesPresence = false;
    SessionSettings.NumPublicConnections = MaxPlayers;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bAllowJoinViaPresence = false;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowInvites = true;
    SessionSettings.bAntiCheatProtected = false;
    SessionSettings.Set(SESSION_GAME_TYPE, FString("true"), EOnlineDataAdvertisementType::ViaOnlineService);

    SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
}

void ULobbySessionManager::DestroySession()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->DestroySession(SESSION_NAME);
    }
}

void ULobbySessionManager::InviteFriends()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem) return;

    FName SubsystemName = OnlineSubsystem->GetSubsystemName();

    if (SubsystemName == TEXT("Steam"))
    {
        IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
        if (ExternalUI.IsValid())
        {
            ExternalUI->ShowFriendsUI(0);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Invite friends not supported for subsystem: %s"), *SubsystemName.ToString());
    }
}

bool ULobbySessionManager::IsSessionHost() const
{
    if (!SessionInterface.IsValid())
    {
        // In offline mode, assume we're always host
        return true;
    }

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SESSION_NAME);
    return Session && Session->bHosting;
}

int32 ULobbySessionManager::GetCurrentPlayerCount() const
{
    if (!SessionInterface.IsValid()) return 0;

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SESSION_NAME);
    return Session ? Session->RegisteredPlayers.Num() : 0;
}

int32 ULobbySessionManager::GetMaxPlayerCount() const
{
    if (!SessionInterface.IsValid()) return 0;

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SESSION_NAME);
    return Session ? Session->SessionSettings.NumPublicConnections : 0;
}

void ULobbySessionManager::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("Create session complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

    OnSessionCreated.Broadcast(bWasSuccessful);

    if (bWasSuccessful)
    {
        // Load lobby map
        UGameplayStatics::OpenLevel(GetWorld(), TEXT("LobbyMap"), true, TEXT("listen"));
    }
}

void ULobbySessionManager::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("Destroy session complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

    OnSessionDestroyed.Broadcast(bWasSuccessful);
}

void ULobbySessionManager::FindSessions()
{
    if (!SessionInterface.IsValid()) return;

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = 20;
    SessionSearch->PingBucketSize = 50;

    // Search for sessions with our game type
    SessionSearch->QuerySettings.Set(SESSION_GAME_TYPE, FString("true"), EOnlineComparisonOp::Equals);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void ULobbySessionManager::JoinSession(int32 SessionIndex)
{
    if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;

    if (SessionIndex >= 0 && SessionIndex < SessionSearch->SearchResults.Num())
    {
        SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[SessionIndex]);
    }
}

void ULobbySessionManager::OnFindSessionsComplete(bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("Find sessions complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

    if (bWasSuccessful && SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Found %d sessions"), SessionSearch->SearchResults.Num());

        for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
        {
            const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];
            UE_LOG(LogTemp, Log, TEXT("Session %d: %s (%d/%d players)"),
                i,
                *Result.Session.OwningUserName,
                Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections,
                Result.Session.SessionSettings.NumPublicConnections);
        }
    }
}

void ULobbySessionManager::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(LogTemp, Log, TEXT("Join session complete: %s"),
        Result == EOnJoinSessionCompleteResult::Success ? TEXT("Success") : TEXT("Failed"));

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        // Get the connection string and travel to the session
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
            }
        }
    }
}