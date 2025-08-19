
#include "MenuCameraManager.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

const float UMenuCameraManager::TRANSITION_UPDATE_RATE = 0.016f;
const FString UMenuCameraManager::LOBBY_CAMERA_TAG = TEXT("LobbyCamera");
const FString UMenuCameraManager::MENU_CAMERA_TAG = TEXT("MenuCamera");
const FString UMenuCameraManager::GAMEPLAY_CAMERA_TAG = TEXT("GameplayCamera");

UMenuCameraManager::UMenuCameraManager()
{
    // Set default values
    bTransitionInProgress = false;
    TransitionAlpha = 0.0f;
    CurrentTransitionName = TEXT("");
}

void UMenuCameraManager::Initialize(APlayerController* InPlayerController)
{
    if (!InPlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("MenuCameraManager: PlayerController is null"));
        return;
    }

    PlayerController = InPlayerController;
    CameraManager = PlayerController->PlayerCameraManager;

    if (!CameraManager)
    {
        UE_LOG(LogTemp, Error, TEXT("MenuCameraManager: CameraManager is null"));
        return;
    }

    // Auto-discover cameras in the world
    UWorld* World = PlayerController->GetWorld();
    if (World)
    {
        for (TActorIterator<ACameraActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            ACameraActor* CameraActor = *ActorIterator;
            if (CameraActor && CameraActor->Tags.Num() > 0)
            {
                for (const FName& Tag : CameraActor->Tags)
                {
                    RegisterCamera(Tag.ToString(), CameraActor);
                    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Auto-registered camera '%s'"), *Tag.ToString());
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Initialized with %d cameras"), RegisteredCameras.Num());
}

bool UMenuCameraManager::StartTransition(const FString& TargetCameraTag, const FCameraTransitionSettings& Settings)
{
    ACameraActor* Camera = FindCameraByTag(TargetCameraTag);
    if (!Camera)
    {
        UE_LOG(LogTemp, Warning, TEXT("MenuCameraManager: Camera with tag '%s' not found"), *TargetCameraTag);
        return false;
    }

    return StartTransitionToCamera(Camera, Settings);
}

bool UMenuCameraManager::StartTransitionToCamera(ACameraActor* InTargetCamera, const FCameraTransitionSettings& Settings)
{
    if (!PlayerController || !CameraManager || !InTargetCamera)
    {
        UE_LOG(LogTemp, Error, TEXT("MenuCameraManager: Missing required components for transition"));
        return false;
    }

    if (bTransitionInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("MenuCameraManager: Stopping existing transition"));
        StopTransition(false);
    }

    // Store transition data
    TargetCamera = InTargetCamera;
    CurrentSettings = Settings;
    CurrentTransitionName = InTargetCamera->GetName();

    // Store original camera state
    StoreOriginalCameraState();

    // Get target camera state
    TargetCameraLocation = TargetCamera->GetActorLocation();
    TargetCameraRotation = TargetCamera->GetActorRotation();

    // Initialize transition
    bTransitionInProgress = true;
    TransitionAlpha = 0.0f;

    // Start fade effect if enabled
    if (CurrentSettings.bUseFadeEffect)
    {
        StartFadeEffect(false); // Fade out
    }

    // Start transition timer
    UWorld* World = PlayerController->GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(TransitionTimer,
            this, &UMenuCameraManager::UpdateTransition,
            TRANSITION_UPDATE_RATE, true);
    }

    // Broadcast event
    OnTransitionStarted.Broadcast(CurrentTransitionName);

    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Started transition to '%s'"), *CurrentTransitionName);
    return true;
}

void UMenuCameraManager::UpdateTransition()
{
    if (!bTransitionInProgress || !PlayerController || !CameraManager)
    {
        return;
    }

    // Update alpha
    float DeltaTime = PlayerController->GetWorld()->GetDeltaSeconds();
    TransitionAlpha += DeltaTime / CurrentSettings.Duration;

    // Apply easing
    float EasedAlpha = ApplyEasingFunction(TransitionAlpha, CurrentSettings.TransitionType);

    // Interpolate camera position and rotation
    FVector CurrentLocation = FMath::Lerp(OriginalCameraLocation, TargetCameraLocation, EasedAlpha);
    FRotator CurrentRotation = FMath::Lerp(OriginalCameraRotation, TargetCameraRotation, EasedAlpha);

    // Apply camera transform
    ApplyCameraTransform(CurrentLocation, CurrentRotation);

    // Check if transition is complete
    if (TransitionAlpha >= 1.0f)
    {
        CompleteTransition();
    }
}

void UMenuCameraManager::CompleteTransition()
{
    if (!PlayerController || !CameraManager)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Completing transition to '%s'"), *CurrentTransitionName);

    // Stop transition timer
    UWorld* World = PlayerController->GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(TransitionTimer);
    }

    // Set final camera position
    if (TargetCamera)
    {
        PlayerController->SetViewTargetWithBlend(TargetCamera, 0.2f, VTBlend_EaseInOut);
    }

    // Start fade in if enabled
    if (CurrentSettings.bUseFadeEffect)
    {
        StartFadeEffect(true); // Fade in
    }

    // Reset transition state
    bTransitionInProgress = false;
    TransitionAlpha = 1.0f;

    // Broadcast completion event
    OnTransitionComplete.Broadcast(true);

    // Clear transition name
    CurrentTransitionName = TEXT("");
}

void UMenuCameraManager::StopTransition(bool bSnapToTarget)
{
    if (!bTransitionInProgress)
    {
        return;
    }

    UWorld* World = PlayerController ? PlayerController->GetWorld() : nullptr;
    if (World)
    {
        World->GetTimerManager().ClearTimer(TransitionTimer);
        World->GetTimerManager().ClearTimer(FadeTimer);
    }

    if (bSnapToTarget && TargetCamera && PlayerController)
    {
        PlayerController->SetViewTargetWithBlend(TargetCamera, 0.0f);
    }

    // Clear fade effect
    if (CameraManager)
    {
        CameraManager->StopCameraFade();
    }

    bTransitionInProgress = false;
    OnTransitionComplete.Broadcast(!bSnapToTarget);

    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Transition stopped"));
}

void UMenuCameraManager::ResetToOriginalCamera()
{
    if (!PlayerController || !CameraManager)
    {
        return;
    }

    StopTransition(false);

    // Reset to original camera position
    PlayerController->SetControlRotation(OriginalCameraRotation);

    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Reset to original camera"));
}

ACameraActor* UMenuCameraManager::FindCameraByTag(const FString& CameraTag)
{
    if (RegisteredCameras.Contains(CameraTag))
    {
        return RegisteredCameras[CameraTag];
    }

    // Try to find in world if not registered
    UWorld* World = PlayerController ? PlayerController->GetWorld() : nullptr;
    if (World)
    {
        for (TActorIterator<ACameraActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            ACameraActor* CameraActor = *ActorIterator;
            if (CameraActor && CameraActor->Tags.Contains(*CameraTag))
            {
                // Auto-register found camera
                RegisterCamera(CameraTag, CameraActor);
                return CameraActor;
            }
        }
    }

    return nullptr;
}

void UMenuCameraManager::RegisterCamera(const FString& CameraName, ACameraActor* Camera)
{
    if (!Camera)
    {
        UE_LOG(LogTemp, Warning, TEXT("MenuCameraManager: Attempted to register null camera '%s'"), *CameraName);
        return;
    }

    RegisteredCameras.Add(CameraName, Camera);
    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Registered camera '%s'"), *CameraName);
}

ACameraActor* UMenuCameraManager::CreateFallbackCamera(const FString& CameraName, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = PlayerController ? PlayerController->GetWorld() : nullptr;
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("MenuCameraManager: Cannot create fallback camera - no world"));
        return nullptr;
    }

    ACameraActor* NewCamera = World->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(),
        Location,
        Rotation
    );

    if (NewCamera)
    {
        NewCamera->Tags.Add(*CameraName);
        RegisterCamera(CameraName, NewCamera);
        UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Created fallback camera '%s'"), *CameraName);
    }

    return NewCamera;
}

float UMenuCameraManager::ApplyEasingFunction(float Alpha, ECameraTransitionType EasingType)
{
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    switch (EasingType)
    {
    case ECameraTransitionType::Linear:
        return Alpha;

    case ECameraTransitionType::EaseIn:
        return Alpha * Alpha;

    case ECameraTransitionType::EaseOut:
        return 1.0f - FMath::Pow(1.0f - Alpha, 2.0f);

    case ECameraTransitionType::EaseInOut:
        return Alpha < 0.5f ?
            2.0f * Alpha * Alpha :
            1.0f - FMath::Pow(-2.0f * Alpha + 2.0f, 2.0f) / 2.0f;

    case ECameraTransitionType::SmoothStep:
        return FMath::SmoothStep(0.0f, 1.0f, Alpha);

    case ECameraTransitionType::Custom:
        if (CurrentSettings.CustomCurve)
        {
            return CurrentSettings.CustomCurve->GetFloatValue(Alpha);
        }
        return FMath::SmoothStep(0.0f, 1.0f, Alpha); // Fallback

    default:
        return Alpha;
    }
}

void UMenuCameraManager::StartFadeEffect(bool bFadeIn)
{
    if (!CameraManager)
    {
        return;
    }

    float FromAlpha = bFadeIn ? 1.0f : 0.0f;
    float ToAlpha = bFadeIn ? 0.0f : 1.0f;

    CameraManager->StartCameraFade(
        FromAlpha, ToAlpha,
        CurrentSettings.FadeDuration,
        CurrentSettings.FadeColor,
        CurrentSettings.bFadeAudio,
        true // Hold when finished
    );

    UE_LOG(LogTemp, Log, TEXT("MenuCameraManager: Started fade effect (FadeIn: %s)"),
        bFadeIn ? TEXT("true") : TEXT("false"));
}

void UMenuCameraManager::StoreOriginalCameraState()
{
    if (!CameraManager)
    {
        return;
    }

    OriginalCameraLocation = CameraManager->GetCameraLocation();
    OriginalCameraRotation = CameraManager->GetCameraRotation();
}

void UMenuCameraManager::ApplyCameraTransform(const FVector& Location, const FRotator& Rotation)
{
    if (!PlayerController)
    {
        return;
    }

    PlayerController->SetControlRotation(Rotation);
    // Note: Location is handled by the camera manager automatically
}

// === PRESET TRANSITIONS ===
bool UMenuCameraManager::TransitionToLobby(const FCameraTransitionSettings& Settings)
{
    return StartTransition(LOBBY_CAMERA_TAG, Settings);
}

bool UMenuCameraManager::TransitionToMainMenu(const FCameraTransitionSettings& Settings)
{
    return StartTransition(MENU_CAMERA_TAG, Settings);
}

bool UMenuCameraManager::TransitionToGameplay(const FCameraTransitionSettings& Settings)
{
    return StartTransition(GAMEPLAY_CAMERA_TAG, Settings);
}