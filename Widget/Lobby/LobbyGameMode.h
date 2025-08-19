#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyGameMode.generated.h"

class ULobbyWidget;

UCLASS()
class HORRORGAME_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	// Game Control Functions
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool AreAllPlayersReady() const;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void PrintLobbyDebugInfo();

	// UI Update Functions
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdateLobbyUI();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	TArray<FString> GetConnectedPlayerNames() const;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	TArray<bool> GetPlayerHostStatus() const;

protected:
	// Setup Functions
	void InitializeSessionInterface();
	void CreateLobbyWidget();

public:
	// Configuration Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Settings")
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Settings")
	FString GameMapName = TEXT("GameLevel");

protected:
	// Widget Reference
	UPROPERTY()
	ULobbyWidget* LobbyWidget;

	// Online Session Interface
	IOnlineSessionPtr SessionInterface;
};