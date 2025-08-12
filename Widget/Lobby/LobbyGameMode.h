#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyGameMode.generated.h"

class ULobbyWidget;
class AHorrorGameState;

UCLASS()
class HORRORGAME_API ALobbyGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ALobbyGameMode();

    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // Start the main game
    UFUNCTION(BlueprintCallable)
    void StartGame();

    // Check if all players are ready (if you want ready system)
    UFUNCTION(BlueprintCallable)
    bool AreAllPlayersReady() const;

protected:
    // Lobby Widget Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    TSubclassOf<ULobbyWidget> LobbyWidgetClass;

    // Current lobby widget instance
    UPROPERTY()
    ULobbyWidget* LobbyWidget;

    // Game map to load when starting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    FString GameMapName = TEXT("L_horrorGame");

private:
    // Session interface
    IOnlineSessionPtr SessionInterface;

    // Initialize session interface
    void InitializeSessionInterface();

    // Update lobby UI with current players
    void UpdateLobbyUI();

    // Get all connected player names
    TArray<FString> GetConnectedPlayerNames() const;

    // Get host status for each player
    TArray<bool> GetPlayerHostStatus() const;
};