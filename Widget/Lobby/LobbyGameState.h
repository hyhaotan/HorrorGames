#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HorrorGame/Data/LobbyType.h"
#include "LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyStateChanged, ELobbyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FLobbyPlayerData&, PlayerData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, const FLobbyPlayerData&, PlayerData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReadyChanged, const FLobbyPlayerData&, PlayerData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownChanged, int32, SecondsLeft);

UCLASS()
class HORRORGAME_API ALobbyGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ALobbyGameState();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // Replicated Properties
    UPROPERTY(Replicated, BlueprintReadOnly)
    ELobbyState CurrentLobbyState;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FLobbySettings LobbySettings;

    UPROPERTY(Replicated, BlueprintReadOnly)
    TArray<FLobbyPlayerData> ConnectedPlayers;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 CountdownTimer;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FString HostSteamID;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnLobbyStateChanged OnLobbyStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerJoined OnPlayerJoined;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerLeft OnPlayerLeft;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerReadyChanged OnPlayerReadyChanged;

    UPROPERTY(BlueprintAssignable)
    FOnCountdownChanged OnCountdownChanged;

    // Public Functions
    UFUNCTION(BlueprintCallable)
    FLobbyPlayerData FindPlayerBySteamID(const FString& SteamID);

    UFUNCTION(BlueprintCallable)
    bool IsPlayerHost(const FString& SteamID);

    UFUNCTION(BlueprintCallable)
    int32 GetReadyPlayerCount();

    UFUNCTION(BlueprintCallable)
    bool CanStartGame();

    UFUNCTION(BlueprintCallable)
    TArray<FLobbyPlayerData> GetPlayersInSlotOrder();

    // Server Functions
    UFUNCTION(Server, Reliable)
    void ServerSetPlayerReady(const FString& SteamID, bool bReady);

    UFUNCTION(Server, Reliable)
    void ServerStartCountdown();

    UFUNCTION(Server, Reliable)
    void ServerCancelCountdown();

    UFUNCTION(Server, Reliable)
    void ServerUpdateLobbySettings(const FLobbySettings& NewSettings);

    // RPC Functions
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUpdateLobbyState(ELobbyState NewState);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayerJoined(const FLobbyPlayerData& PlayerData);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayerLeft(const FLobbyPlayerData& PlayerData);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayerReadyChanged(const FLobbyPlayerData& PlayerData);

    void UpdateLobbyState(ELobbyState NewState);
protected:
    // Internal Functions

    void StartCountdownTimer();
    void UpdateCountdown();
    void HandleCountdownFinished();

    FTimerHandle CountdownTimerHandle;
    const int32 COUNTDOWN_DURATION = 5; // seconds
};