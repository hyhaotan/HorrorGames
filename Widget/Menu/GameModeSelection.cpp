// GameModeSelection.cpp - Updated version
#include "GameModeSelection.h"
#include "HorrorGame/Widget/Menu/MainMenu.h"
#include "HorrorGame/Widget/Lobby/LobbyWidget.h"
#include "HorrorGame/Widget/Lobby/LobbyGameMode.h"
#include "HorrorGame/Object/LobbySessionManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include <Online/OnlineSessionNames.h>

void UGameModeSelection::NativeConstruct()
{
    Super::NativeConstruct();

    // Play show animation if available
    if (ShowAnim)
    {
        PlayAnimation(ShowAnim);
    }

    // Bind button events
    if (SinglePlayerButton)
    {
        SinglePlayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnSinglePlayerClicked);
    }
    if (MultiplayerButton)
    {
        MultiplayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnMultiplayerClicked);
    }
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnBackClicked);
    }

    // Bind hide animation finished event
    if (HideAnim)
    {
        FWidgetAnimationDynamicEvent AnimationFinished;
        AnimationFinished.BindDynamic(this, &UGameModeSelection::OnHideAnimationFinished);
        BindToAnimationFinished(HideAnim, AnimationFinished);
    }

    // Create and initialize session manager
    SessionManager = NewObject<ULobbySessionManager>(this);
    if (SessionManager)
    {
        // Bind session events
        SessionManager->OnSessionCreated.AddDynamic(this, &UGameModeSelection::OnSessionCreated);
    }
}

void UGameModeSelection::OnSinglePlayerClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Single Player selected"));

    PendingState = ETransitionState::Single;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        // Directly load single player game
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"));
    }
}

void UGameModeSelection::OnMultiplayerClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Multiplayer selected"));

    PendingState = ETransitionState::Multiplayer;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        CreateMultiplayerLobby();
    }
}

void UGameModeSelection::CreateMultiplayerLobby()
{
    if (SessionManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Creating multiplayer lobby session..."));

        // Create a 4-player lobby session (like Dead by Daylight)
        SessionManager->CreateLobbySession(4);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SessionManager is null!"));
    }
}

void UGameModeSelection::OnSessionCreated(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Lobby session created successfully"));
        // Session manager will automatically load the lobby map
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create lobby session"));
        // Show error message to user or fallback to main menu
        ShowConnectionError();
    }
}

void UGameModeSelection::ShowConnectionError()
{
    // You can implement error UI here
    UE_LOG(LogTemp, Warning, TEXT("Connection failed - returning to main menu"));

    // Return to main menu on error
    if (MainMenuClass)
    {
        RemoveFromParent();
        UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
        if (Menu)
        {
            Menu->AddToViewport();
        }
    }
}

void UGameModeSelection::OnBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Back button clicked"));

    PendingState = ETransitionState::Back;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        ReturnToMainMenu();
    }
}

void UGameModeSelection::ReturnToMainMenu()
{
    if (MainMenuClass)
    {
        RemoveFromParent();
        UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
        if (Menu)
        {
            Menu->AddToViewport();
        }
    }
}

void UGameModeSelection::OnHideAnimationFinished()
{
    switch (PendingState)
    {
    case ETransitionState::Single:
        UE_LOG(LogTemp, Log, TEXT("Transitioning to single player game"));
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"));
        break;

    case ETransitionState::Multiplayer:
        UE_LOG(LogTemp, Log, TEXT("Transitioning to multiplayer lobby"));
        CreateMultiplayerLobby();
        break;

    case ETransitionState::Back:
        UE_LOG(LogTemp, Log, TEXT("Returning to main menu"));
        ReturnToMainMenu();
        break;

    default:
        break;
    }

    // Reset pending state
    PendingState = ETransitionState::None;
}

void UGameModeSelection::OnShowAnimationFinished()
{
    // Animation finished, ready for user input
    UE_LOG(LogTemp, Log, TEXT("Show animation finished"));
}