#include "LobbyGameMode.h"
#include "LobbyWidget.h"
#include "HorrorGame/Core/HorrorGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"

ALobbyGameMode::ALobbyGameMode()
{
    bUseSeamlessTravel = true;
    GameStateClass = AHorrorGameState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    InitializeSessionInterface();

    // Create lobby widget
    if (LobbyWidgetClass)
    {
        APlayerController* FirstPC = GetWorld()->GetFirstPlayerController();
        if (FirstPC)
        {
            LobbyWidget = CreateWidget<ULobbyWidget>(FirstPC, LobbyWidgetClass);
            if (LobbyWidget)
            {
                LobbyWidget->AddToViewport();
                UE_LOG(LogTemp, Log, TEXT("Lobby widget created and added to viewport"));
            }
        }
    }

    UpdateLobbyUI();
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
        UE_LOG(LogTemp, Warning, TEXT("LobbyGameMode: No OSS available"));
    }
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    FString PlayerName = NewPlayer->PlayerState ?
        NewPlayer->PlayerState->GetPlayerName() : TEXT("Unknown Player");

    UE_LOG(LogTemp, Log, TEXT("Player joined lobby: %s"), *PlayerName);

    // Update UI
    UpdateLobbyUI();

    if (LobbyWidget && NewPlayer->PlayerState)
    {
        bool bIsHost = (GameState->PlayerArray.Num() == 1);
        LobbyWidget->OnPlayerJoined(PlayerName, bIsHost);
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    FString PlayerName = Exiting->PlayerState ?
        Exiting->PlayerState->GetPlayerName() : TEXT("Unknown Player");

    UE_LOG(LogTemp, Log, TEXT("Player left lobby: %s"), *PlayerName);

    if (LobbyWidget && Exiting->PlayerState)
    {
        LobbyWidget->OnPlayerLeft(PlayerName);
    }

    Super::Logout(Exiting);
    UpdateLobbyUI();
}

void ALobbyGameMode::StartGame()
{
    UE_LOG(LogTemp, Log, TEXT("Starting game from lobby"));

    // Transition all players to the main game level
    GetWorld()->ServerTravel(GameMapName + TEXT("?listen"));
}

bool ALobbyGameMode::AreAllPlayersReady() const
{
    // If you implement a ready system, check here
    // For now, just check if we have at least one player
    return GameState && GameState->PlayerArray.Num() > 0;
}

void ALobbyGameMode::UpdateLobbyUI()
{
    if (!LobbyWidget) return;

    TArray<FString> PlayerNames = GetConnectedPlayerNames();
    TArray<bool> HostStatus = GetPlayerHostStatus();

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

    return Names;
}

TArray<bool> ALobbyGameMode::GetPlayerHostStatus() const
{
    TArray<bool> HostStatus;

    if (GameState)
    {
        for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
        {
            // First player (index 0) is typically the host
            HostStatus.Add(i == 0);
        }
    }

    return HostStatus;
}