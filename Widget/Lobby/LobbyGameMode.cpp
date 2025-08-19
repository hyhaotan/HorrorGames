#include "LobbyGameMode.h"
#include "LobbyWidget.h"
#include "HorrorGame/Core/HorrorGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Engine/World.h"

ALobbyGameMode::ALobbyGameMode()
{
    // Enable seamless travel for smooth transitions
    bUseSeamlessTravel = true;

    // Set custom game state
    GameStateClass = AHorrorGameState::StaticClass();

    // Default player class
    DefaultPawnClass = nullptr; // No pawn needed in lobby
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("LobbyGameMode: BeginPlay"));

    // Initialize session interface
    InitializeSessionInterface();

    // Create lobby widget after a small delay to ensure everything is loaded
    FTimerHandle LobbyTimer;
    GetWorld()->GetTimerManager().SetTimer(LobbyTimer, [this]()
        {
            CreateLobbyWidget();
        }, 0.1f, false);
}

void ALobbyGameMode::InitializeSessionInterface()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        UE_LOG(LogTemp, Log, TEXT("LobbyGameMode using OSS: %s"),
            *OnlineSubsystem->GetSubsystemName().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("LobbyGameMode: No OSS available - offline mode"));
    }
}

void ALobbyGameMode::CreateLobbyWidget()
{
    if (!LobbyWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("LobbyWidgetClass is not set!"));
        return;
    }

    // Get first player controller
    APlayerController* FirstPC = GetWorld()->GetFirstPlayerController();
    if (!FirstPC)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerController found!"));
        return;
    }

    // Create lobby widget
    LobbyWidget = CreateWidget<ULobbyWidget>(FirstPC, LobbyWidgetClass);
    if (LobbyWidget)
    {
        LobbyWidget->AddToViewport(10); // High Z-order

        UE_LOG(LogTemp, Log, TEXT("Lobby widget created and added to viewport"));

        // Initial UI update
        UpdateLobbyUI();

        // Show mouse cursor for UI interaction
        FirstPC->bShowMouseCursor = true;
        FirstPC->bEnableClickEvents = true;
        FirstPC->bEnableMouseOverEvents = true;

        // Set input mode to UI only
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
        FirstPC->SetInputMode(InputMode);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create lobby widget"));
    }
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer || !NewPlayer->PlayerState)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid player or player state in PostLogin"));
        return;
    }

    FString PlayerName = NewPlayer->PlayerState->GetPlayerName();
    UE_LOG(LogTemp, Log, TEXT("Player joined lobby: %s"), *PlayerName);

    // Update game state player count
    if (AHorrorGameState* HorrorState = GetGameState<AHorrorGameState>())
    {
        HorrorState->CurrentPlayers = GameState->PlayerArray.Num();
        UE_LOG(LogTemp, Log, TEXT("Updated player count: %d"), HorrorState->CurrentPlayers);
    }

    // Update lobby UI
    UpdateLobbyUI();

    // Notify lobby widget of new player
    if (LobbyWidget)
    {
        bool bIsHost = (GameState->PlayerArray.Num() == 1); // First player is host
        LobbyWidget->OnPlayerJoined(PlayerName, bIsHost);
    }

    // Setup input for new player
    if (NewPlayer == GetWorld()->GetFirstPlayerController())
    {
        NewPlayer->bShowMouseCursor = true;
        NewPlayer->bEnableClickEvents = true;
        NewPlayer->bEnableMouseOverEvents = true;
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    if (Exiting && Exiting->PlayerState)
    {
        FString PlayerName = Exiting->PlayerState->GetPlayerName();
        UE_LOG(LogTemp, Log, TEXT("Player left lobby: %s"), *PlayerName);

        // Notify lobby widget
        if (LobbyWidget)
        {
            LobbyWidget->OnPlayerLeft(PlayerName);
        }
    }

    Super::Logout(Exiting);

    // Update game state
    if (AHorrorGameState* HorrorState = GetGameState<AHorrorGameState>())
    {
        HorrorState->CurrentPlayers = FMath::Max(0, GameState->PlayerArray.Num() - 1);
    }

    // Update UI
    UpdateLobbyUI();
}

void ALobbyGameMode::StartGame()
{
    int32 PlayerCount = GameState ? GameState->PlayerArray.Num() : 0;
    UE_LOG(LogTemp, Log, TEXT("Starting game with %d players"), PlayerCount);

    if (PlayerCount < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough players to start game"));
        return;
    }

    // Disable lobby input
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
    }

    // Seamless travel to game map
    GetWorld()->ServerTravel(GameMapName + TEXT("?listen"), true);
}

bool ALobbyGameMode::AreAllPlayersReady() const
{
    // Simple implementation - just check if we have players
    return GameState && GameState->PlayerArray.Num() > 0;
}

void ALobbyGameMode::UpdateLobbyUI()
{
    if (!LobbyWidget || !GameState) return;

    // Get current player info
    TArray<FString> PlayerNames = GetConnectedPlayerNames();
    TArray<bool> HostStatus = GetPlayerHostStatus();

    // Update lobby widget
    LobbyWidget->UpdatePlayerSlots(PlayerNames, HostStatus);
}

TArray<FString> ALobbyGameMode::GetConnectedPlayerNames() const
{
    TArray<FString> Names;

    if (GameState)
    {
        for (APlayerState* PlayerState : GameState->PlayerArray)
        {
            if (PlayerState)
            {
                Names.Add(PlayerState->GetPlayerName());
            }
        }
    }

    // Pad with empty strings to fill all slots
    while (Names.Num() < 4)
    {
        Names.Add(TEXT(""));
    }

    return Names;
}

TArray<bool> ALobbyGameMode::GetPlayerHostStatus() const
{
    TArray<bool> HostStatus;

    if (GameState)
    {
        for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
        {
            // First player (index 0) is host
            HostStatus.Add(i == 0);
        }
    }

    // Pad with false values
    while (HostStatus.Num() < 4)
    {
        HostStatus.Add(false);
    }

    return HostStatus;
}

// Utility function for debugging
void ALobbyGameMode::PrintLobbyDebugInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== LOBBY DEBUG INFO ==="));
    UE_LOG(LogTemp, Log, TEXT("Player Count: %d"), GameState ? GameState->PlayerArray.Num() : 0);

    if (GameState)
    {
        for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
        {
            APlayerState* PS = GameState->PlayerArray[i];
            if (PS)
            {
                UE_LOG(LogTemp, Log, TEXT("Player %d: %s (Host: %s)"),
                    i, *PS->GetPlayerName(), i == 0 ? TEXT("Yes") : TEXT("No"));
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Session Interface Valid: %s"),
        SessionInterface.IsValid() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Lobby Widget Valid: %s"),
        LobbyWidget ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("======================"));
}