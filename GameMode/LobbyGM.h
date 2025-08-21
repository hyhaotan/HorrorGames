#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorGame/Data/LobbyType.h"
#include "LobbyGM.generated.h"

class ALobbyPlayerPlatform;
class UPlayerIDManager;
class ALobbyGameState;
class ALobbyPlayerController;

UCLASS()
class HORRORGAME_API ALobbyGM : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALobbyGM();

protected:
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

public:
    // Platform Management
    UPROPERTY(BlueprintReadOnly)
    TArray<ALobbyPlayerPlatform*> Platforms;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
    TSubclassOf<APawn> LobbyCharacterClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
    FLobbySettings DefaultLobbySettings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
    FString GameLevelToLoad;

    // Player Manager
    UPROPERTY(BlueprintReadOnly)
    UPlayerIDManager* PlayerIDManager;

    // Public Functions
    UFUNCTION(BlueprintCallable)
    bool StartLobbyGame();

    UFUNCTION(BlueprintCallable)
    void UpdateLobbySettings(const FLobbySettings& NewSettings);

    UFUNCTION(BlueprintCallable)
    bool CanPlayerJoin(APlayerController* NewPlayer);

    UFUNCTION(BlueprintCallable)
    void KickPlayer(const FString& SteamID);

protected:
    // Internal Functions
    void SetupPlatforms();
    ALobbyPlayerPlatform* FindFreePlatform() const;
    void AssignPlayerToPlatform(ALobbyPlayerController* Player, ALobbyPlayerPlatform* Platform);
    void RemovePlayerFromPlatform(APlayerController* Player);

    void RegisterNewPlayer(ALobbyPlayerController* Player);
    void UpdateGameStateWithPlayer(const FLobbyPlayerData& PlayerData, bool bAdding = true);

    bool ValidatePlayerConnection(APlayerController* NewPlayer);
    void HandlePlayerRegistrationComplete(ALobbyPlayerController* Player, bool bSuccess);

private:
    TMap<APlayerController*, ALobbyPlayerPlatform*> PlayerToPlatform;
    TMap<FString, ALobbyPlayerController*> SteamIDToPlayerController;

    ALobbyGameState* LobbyGameState;
    FString HostSteamID;
    bool bIsGameStarting;
};