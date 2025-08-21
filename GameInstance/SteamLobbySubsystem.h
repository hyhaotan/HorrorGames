#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "HorrorGame/Data/LobbyType.h"
#include "SteamLobbySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCreated, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyJoined, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbySearchComplete, bool, bSuccess, const TArray<FString>&, LobbyList);

UCLASS()
class HORRORGAME_API USteamLobbySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnLobbyCreated OnLobbyCreated;

    UPROPERTY(BlueprintAssignable)
    FOnLobbyJoined OnLobbyJoined;

    UPROPERTY(BlueprintAssignable)
    FOnLobbySearchComplete OnLobbySearchComplete;

    // Lobby Management
    UFUNCTION(BlueprintCallable)
    void CreateLobby(const FLobbySettings& Settings);

    UFUNCTION(BlueprintCallable)
    void JoinLobby(const FString& LobbyID, const FString& Password = TEXT(""));

    UFUNCTION(BlueprintCallable)
    void LeaveLobby();

    UFUNCTION(BlueprintCallable)
    void SearchLobbies();

    UFUNCTION(BlueprintCallable)
    void InvitePlayerToLobby(const FString& FriendSteamID);

    // Getters
    UFUNCTION(BlueprintCallable)
    bool IsInLobby() const { return bIsInLobby; }

    UFUNCTION(BlueprintCallable)
    FString GetCurrentLobbyID() const { return CurrentLobbyID; }

protected:
    // Session Delegates
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    FString CurrentLobbyID;
    bool bIsInLobby;
    FLobbySettings CurrentLobbySettings;

    // Delegate Handles
    FDelegateHandle CreateSessionCompleteHandle;
    FDelegateHandle JoinSessionCompleteHandle;
    FDelegateHandle FindSessionsCompleteHandle;
    FDelegateHandle DestroySessionCompleteHandle;
};