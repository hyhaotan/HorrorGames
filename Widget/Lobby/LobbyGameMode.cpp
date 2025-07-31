#include "LobbyGameMode.h"
#include "LobbyWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameState.h"
#include "HorrorGame/Core/HorrorGameState.h"

ALobbyGameMode::ALobbyGameMode()
{
    // Set default values
    bUseSeamlessTravel = true;

    // Set the game state class to our custom game state
    GameStateClass = AHorrorGameState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Create and show the lobby widget
    if (LobbyWidgetClass)
    {
        LobbyWidget = CreateWidget<ULobbyWidget>(GetWorld()->GetFirstPlayerController(), LobbyWidgetClass);
        if (LobbyWidget)
        {
            LobbyWidget->AddToViewport();
        }
    }

    UpdateMaxPlayers();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // Update player count
    AHorrorGameState* HorrorState = GetGameState<AHorrorGameState>();
    if (HorrorState)
    {
        HorrorState->CurrentPlayers++;
    }

    // Update lobby UI when a new player joins
    if (LobbyWidget)
    {
        TArray<FUniqueNetIdRepl> PlayerIds;
        for (APlayerState* PlayerState : GameState->PlayerArray)
        {
            if (PlayerState && PlayerState->GetUniqueId().IsValid())
            {
                PlayerIds.Add(PlayerState->GetUniqueId());
            }
        }
        LobbyWidget->UpdatePlayerSlots(PlayerIds);
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    // Update player count before calling parent
    AHorrorGameState* HorrorState = GetGameState<AHorrorGameState>();
    if (HorrorState)
    {
        HorrorState->CurrentPlayers--;
    }

    Super::Logout(Exiting);

    // Update lobby UI when a player leaves
    if (LobbyWidget)
    {
        TArray<FUniqueNetIdRepl> PlayerIds;
        for (APlayerState* PlayerState : GameState->PlayerArray)
        {
            if (PlayerState && PlayerState->GetUniqueId().IsValid() && PlayerState != Exiting->PlayerState)
            {
                PlayerIds.Add(PlayerState->GetUniqueId());
            }
        }
        LobbyWidget->UpdatePlayerSlots(PlayerIds);
    }
}

void ALobbyGameMode::UpdateMaxPlayers()
{
    // Get the session interface to access session settings
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(TEXT("Steam"));
    if (OnlineSubsystem)
    {
        IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
            if (Session)
            {
                // Update max players in game state
                AHorrorGameState* HorrorState = GetGameState<AHorrorGameState>();
                if (HorrorState)
                {
                    HorrorState->SetMaxPlayers(Session->SessionSettings.NumPublicConnections);
                }
            }
        }
    }
}