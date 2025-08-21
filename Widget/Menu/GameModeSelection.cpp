#include "GameModeSelection.h"
#include "HorrorGame/Widget/Menu/MainMenu.h"
#include "HorrorGame/Widget/Lobby/LobbyMainWidget.h"
#include "HorrorGame/Object/LobbySessionManager.h"
#include "HorrorGame/Object/MenuCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "Engine/Engine.h"

void UGameModeSelection::NativeConstruct()
{
    Super::NativeConstruct();

    // Play show animation if available and bind finished event
    if (ShowAnim)
    {
        PlayAnimation(ShowAnim);

        FWidgetAnimationDynamicEvent ShowFinished;
        ShowFinished.BindDynamic(this, &UGameModeSelection::OnShowAnimationFinished);
        BindToAnimationFinished(ShowAnim, ShowFinished);
    }

    // Bind hide animation finished event
    if (HideAnim)
    {
        FWidgetAnimationDynamicEvent HideFinished;
        HideFinished.BindDynamic(this, &UGameModeSelection::OnHideAnimationFinished);
        BindToAnimationFinished(HideAnim, HideFinished);
    }

    // Buttons
    BindButtonEvents();

    // Initialize camera manager and session manager
    InitializeCameraManager();

    SessionManager = NewObject<ULobbySessionManager>(this);
    if (SessionManager)
    {
        SessionManager->OnSessionCreated.AddDynamic(this, &UGameModeSelection::OnSessionCreated);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: Could not create SessionManager"));
    }

    PendingState = ETransitionState::None;
}

void UGameModeSelection::BindButtonEvents()
{
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
}

void UGameModeSelection::InitializeCameraManager()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeSelection: PlayerController is null"));
        return;
    }

    // Create menu camera manager instance and initialize it
    CameraManager = NewObject<UMenuCameraManager>(this);
    if (!CameraManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeSelection: Failed to create CameraManager"));
        return;
    }

    CameraManager->Initialize(PC);

    // Bind camera manager events
    CameraManager->OnTransitionStarted.AddDynamic(this, &UGameModeSelection::OnCameraTransitionStarted);
    CameraManager->OnTransitionComplete.AddDynamic(this, &UGameModeSelection::OnCameraTransitionComplete);

    UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Camera manager initialized."));
}

void UGameModeSelection::SetButtonsEnabled(bool bEnabled)
{
    if (SinglePlayerButton) SinglePlayerButton->SetIsEnabled(bEnabled);
    if (MultiplayerButton) MultiplayerButton->SetIsEnabled(bEnabled);
    if (BackButton) BackButton->SetIsEnabled(bEnabled);
}

void UGameModeSelection::OnSinglePlayerClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Single Player selected"));
    PendingState = ETransitionState::Single;

    // Play hide animation if exists so OnHideAnimationFinished performs the action
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
    UE_LOG(LogTemp, Log, TEXT("Multiplayer selected"));
    PendingState = ETransitionState::Multiplayer;

    // Disable buttons while transitioning
    SetButtonsEnabled(false);

    // Start camera transition to lobby; fallback to direct lobby creation on failure
    StartCameraTransitionToLobby();
}

void UGameModeSelection::StartCameraTransitionToLobby()
{
    if (!CameraManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: CameraManager null, creating lobby directly"));
        CreateMultiplayerLobby();
        return;
    }

    // Prepare transition settings structure (match your FCameraTransitionSettings definition)
    FCameraTransitionSettings TransitionSettings;
    TransitionSettings.Duration = CameraTransitionDuration;
    TransitionSettings.TransitionType = CameraManager->CameraTransitionType;
    TransitionSettings.bUseFadeEffect = bUseCameraFade;
    TransitionSettings.FadeDuration = CameraFadeDuration;
    TransitionSettings.FadeColor = CameraFadeColor;
    TransitionSettings.bFadeAudio = bFadeAudioDuringTransition;
    TransitionSettings.CustomCurve = CameraTransitionCurve;

    // Optionally hide this widget (play hide animation) so camera view is exposed
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }

    // Start transition; MenuCameraManager::TransitionToLobby returns true if started
    bool bStarted = CameraManager->TransitionToLobby(TransitionSettings);
    if (!bStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: Camera transition failed to start, creating lobby directly"));
        CreateMultiplayerLobby();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Camera transition started to lobby"));
    }
}

void UGameModeSelection::OnCameraTransitionStarted(const FString TransitionName)
{
    UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Camera transition started -> %s"), *TransitionName);
    SetButtonsEnabled(false);
}

void UGameModeSelection::OnCameraTransitionComplete(bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Camera transition completed (Success: %s)"),
        bWasSuccessful ? TEXT("Yes") : TEXT("No"));

    // If transition succeeded and we intended Multiplayer, create lobby now
    if (bWasSuccessful && PendingState == ETransitionState::Multiplayer)
    {
        CreateMultiplayerLobby();
    }
    else if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: Camera transition failed -> showing error"));
        SetButtonsEnabled(true);
        ShowConnectionError();
    }
}

void UGameModeSelection::CreateMultiplayerLobby()
{
    if (SessionManager)
    {
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Creating multiplayer lobby session..."));
        SessionManager->CreateLobbySession(4);
        // OnSessionCreated delegate will handle showing UI on success/failure.
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: SessionManager null, showing lobby UI directly"));
        ShowLobbyUIDirectly();
    }
}

void UGameModeSelection::ShowLobbyUIDirectly()
{
    if (!LobbyWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeSelection: LobbyWidgetClass not set"));
        return;
    }

    UWorld* World = GetWorld();
    APlayerController* PC = GetOwningPlayer();
    if (!World || !PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: Cannot show lobby widget (no world/player)"));
        return;
    }

    ULobbyMainWidget* LobbyWidget = CreateWidget<ULobbyMainWidget>(PC, LobbyWidgetClass);
    if (LobbyWidget)
    {
        RemoveFromParent();
        LobbyWidget->AddToViewport();
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Lobby UI shown"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeSelection: Failed to create lobby widget"));
    }
}

void UGameModeSelection::OnSessionCreated(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Lobby session created successfully"));
        ShowLobbyUIDirectly();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeSelection: Failed to create lobby session"));
        ShowConnectionError();
    }
}

void UGameModeSelection::ShowConnectionError()
{
    UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: ShowConnectionError triggered"));

    if (CameraManager && CameraManager->IsTransitionActive())
    {
        CameraManager->ResetToOriginalCamera();
    }

    SetButtonsEnabled(true);

    if (bReturnToMenuOnError)
    {
        ReturnToMainMenu();
    }
    else
    {
        // TODO: instantiate/show an error message widget here
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: TODO - show error message widget"));
    }
}

void UGameModeSelection::OnBackClicked()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Back clicked"));

    if (CameraManager && CameraManager->IsTransitionActive())
    {
        CameraManager->StopTransition(false);
    }

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
    if (!MainMenuClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: MainMenuClass not set"));
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameModeSelection: No owning player to create main menu"));
        return;
    }

    RemoveFromParent();
    UMainMenu* Menu = CreateWidget<UMainMenu>(PC, MainMenuClass);
    if (Menu)
    {
        Menu->AddToViewport();
    }
}

void UGameModeSelection::OnHideAnimationFinished()
{
    switch (PendingState)
    {
    case ETransitionState::Single:
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: HideAnim finished -> Open single player level"));
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"));
        break;

    case ETransitionState::Multiplayer:
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: HideAnim finished -> Ensure lobby creation"));
        CreateMultiplayerLobby();
        break;

    case ETransitionState::Back:
        UE_LOG(LogTemp, Log, TEXT("GameModeSelection: HideAnim finished -> Return to main menu"));
        ReturnToMainMenu();
        break;

    default:
        break;
    }

    PendingState = ETransitionState::None;
}

void UGameModeSelection::OnShowAnimationFinished()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeSelection: Show animation finished - ready for input"));
    SetButtonsEnabled(true);
}
