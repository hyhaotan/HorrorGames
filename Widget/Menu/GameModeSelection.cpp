#include "GameModeSelection.h"
#include "HorrorGame/Widget/Menu/MainMenu.h"
#include "HorrorGame/Widget/Lobby/LobbyWidget.h"
#include "HorrorGame/Widget/Lobby/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include <Online/OnlineSessionNames.h>

static const FName SESSION_GAME_TYPE = TEXT("LobbyMap");

void UGameModeSelection::NativeConstruct()
{
    Super::NativeConstruct();

    if (ShowAnim)
    {
        PlayAnimation(ShowAnim);
    }

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

    if (HideAnim)
    {
        FWidgetAnimationDynamicEvent AnimationFinished;
        AnimationFinished.BindDynamic(this, &UGameModeSelection::OnHideAnimationFinished);
        BindToAnimationFinished(HideAnim, AnimationFinished);
    }

    // Initialize Online Subsystem
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("Steam"));
    SessionInterface = OSS ? OSS->GetSessionInterface() : nullptr;
}

void UGameModeSelection::OnSinglePlayerClicked()
{
    PendingState = ETransitionState::Single;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"));
    }
}

void UGameModeSelection::OnMultiplayerClicked()
{
    PendingState = ETransitionState::Multiplayer;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else if (LobbyWidgetClass)
    {
        CreateLobbySession();
    }
}

void UGameModeSelection::CreateLobbySession()
{
    if (!SessionInterface.IsValid()) return;

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.bUsesPresence = true;
    SessionSettings.NumPublicConnections = 4;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bAllowJoinViaPresence = true;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowInvites = true;
    SessionSettings.Set(SESSION_GAME_TYPE, FString("true"), EOnlineDataAdvertisementType::ViaOnlineService);
    
    // Add the game mode class to session settings
    SessionSettings.Set(SETTING_GAMEMODE, ALobbyGameMode::StaticClass()->GetPathName(), 
        EOnlineDataAdvertisementType::ViaOnlineService);

    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
    SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UGameModeSelection::OnCreateSessionComplete);
}

void UGameModeSelection::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        // Load the lobby map after successful session creation
        UGameplayStatics::OpenLevel(this, TEXT("LobbyMap"), true, "listen");
    }
}

void UGameModeSelection::OnBackClicked()
{
    PendingState = ETransitionState::Back;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else if (MainMenuClass)
    {
        RemoveFromParent();
        UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
        Menu->AddToViewport();
    }
}

void UGameModeSelection::OnHideAnimationFinished()
{
    switch (PendingState)
    {
    case ETransitionState::Single:
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"));
        break;
    case ETransitionState::Multiplayer:
        if (LobbyWidgetClass)
        {
            CreateLobbySession();
        }
        break;
    case ETransitionState::Back:
        if (MainMenuClass)
        {
            UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
            Menu->AddToViewport();
        }
        break;
    default:
        break;
    }
    PendingState = ETransitionState::None;
}

void UGameModeSelection::OnShownimationFinished()
{
    PlayAnimation(ShowAnim);
}
