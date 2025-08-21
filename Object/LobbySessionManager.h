#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "LobbySessionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionDestroyed, bool, bWasSuccessful);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoined, FString, PlayerName, bool, bIsHost);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, FString, PlayerName);

UCLASS(BlueprintType)
class HORRORGAME_API ULobbySessionManager : public UObject
{
    GENERATED_BODY()

public:
    ULobbySessionManager();

    // Create a new lobby session
    UFUNCTION(BlueprintCallable)
    void CreateLobbySession(int32 MaxPlayers = 4);

    // Destroy current session
    UFUNCTION(BlueprintCallable)
    void DestroySession();

    // Find and join available sessions
    UFUNCTION(BlueprintCallable)
    void FindSessions();

    // Join a specific session
    UFUNCTION(BlueprintCallable)
    void JoinSession(int32 SessionIndex);

    // Invite friends through Steam overlay
    UFUNCTION(BlueprintCallable)
    void InviteFriends();

    // Check if we're the session host
    UFUNCTION(BlueprintCallable)
    bool IsSessionHost() const;

    // Get current session info
    UFUNCTION(BlueprintCallable)
    int32 GetCurrentPlayerCount() const;

    UFUNCTION(BlueprintCallable)
    int32 GetMaxPlayerCount() const;

    // Delegates
    UPROPERTY(BlueprintAssignable)
    FOnSessionCreated OnSessionCreated;

    UPROPERTY(BlueprintAssignable)
    FOnSessionDestroyed OnSessionDestroyed;

    //UPROPERTY(BlueprintAssignable)
    //FOnPlayerJoined OnPlayerJoined;

    //UPROPERTY(BlueprintAssignable)
    //FOnPlayerLeft OnPlayerLeft;

private:
    // Session interface
    IOnlineSessionPtr SessionInterface;

    // Session delegates
    FDelegateHandle CreateSessionDelegateHandle;
    FDelegateHandle DestroySessionDelegateHandle;
    FDelegateHandle FindSessionsDelegateHandle;
    FDelegateHandle JoinSessionDelegateHandle;

    // Session search results
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    // Callback functions
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    // Initialize the session interface
    void InitializeSessionInterface();

    // Session settings constants
    static const FName SESSION_NAME;
    static const FName SESSION_GAME_TYPE;
};