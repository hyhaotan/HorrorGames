#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorGame/Data/LobbyType.h"
#include "LobbyPlayerController.generated.h"

class UPlayerIDManager;
class ALobbyGameState;
class ULobbyMainWidget;

UCLASS()
class HORRORGAME_API ALobbyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ALobbyPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

public:
    // Player Manager
    UPROPERTY(BlueprintReadOnly)
    UPlayerIDManager* PlayerIDManager;

    // UI
    UPROPERTY(BlueprintReadOnly)
    ULobbyMainWidget* LobbyMainWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<ULobbyMainWidget> LobbyMainWidgetClass;

    // Player Data
    UPROPERTY(BlueprintReadOnly, Replicated)
    FLobbyPlayerData MyPlayerData;

    // Public Functions
    UFUNCTION(BlueprintCallable)
    void SetPlayerReady(bool bReady);

    UFUNCTION(BlueprintCallable)
    void LeaveLobby();

    UFUNCTION(BlueprintCallable)
    void InviteFriend(const FString& FriendSteamID);

    UFUNCTION(Server, Reliable)
    void ServerInviteFriend(const FString& FriendSteamID);

    UFUNCTION(Client, Reliable)
    void ClientReceiveInviteResult(bool bSuccess, const FString& FriendSteamID);

    UFUNCTION(BlueprintCallable)
    void UpdateLobbySettings(const FLobbySettings& NewSettings);

    UFUNCTION(BlueprintCallable)
    void StartGame();

    // Server RPCs
    UFUNCTION(Server, Reliable)
    void ServerRegisterPlayer();

    UFUNCTION(Server, Reliable)
    void ServerSetReady(bool bReady);

    UFUNCTION(Server, Reliable)
    void ServerLeaveLobby();

    UFUNCTION(Server, Reliable)
    void ServerStartGame();

    // Client RPCs
    UFUNCTION(Client, Reliable)
    void ClientShowLobbyUI();

    UFUNCTION(Client, Reliable)
    void ClientUpdatePlayerData(const FLobbyPlayerData& PlayerData);

    UFUNCTION(Client, Reliable)
    void ClientGameStarting();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Internal Functions
    void InitializePlayerManager();
    void CreateLobbyUI();
    void RegisterWithGameState();
    void UpdateMyPlayerData();

    // Event Handlers
    UFUNCTION()
    void OnLobbyStateChanged(ELobbyState NewState);

    UFUNCTION()
    void OnPlayerJoinedLobby(const FLobbyPlayerData& PlayerData);

    UFUNCTION()
    void OnPlayerLeftLobby(const FLobbyPlayerData& PlayerData);

    UFUNCTION()
    void OnPlayerReadyChanged(const FLobbyPlayerData& PlayerData);

    UFUNCTION()
    void OnCountdownChanged(int32 SecondsLeft);

private:
    bool bHasRegistered;
    ALobbyGameState* LobbyGameState;
};