// MenuCameraManager.h - Dedicated Camera Transition System
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Curves/CurveFloat.h"
#include "MenuCameraManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraTransitionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraTransitionStarted, FString, TransitionName);

UENUM(BlueprintType)
enum class ECameraTransitionType : uint8
{
    Linear          UMETA(DisplayName = "Linear"),
    EaseIn          UMETA(DisplayName = "Ease In"),
    EaseOut         UMETA(DisplayName = "Ease Out"),
    EaseInOut       UMETA(DisplayName = "Ease In Out"),
    SmoothStep      UMETA(DisplayName = "Smooth Step"),
    Custom          UMETA(DisplayName = "Custom Curve")
};

USTRUCT(BlueprintType)
struct FCameraTransitionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    ECameraTransitionType TransitionType = ECameraTransitionType::SmoothStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition",
        meta = (EditCondition = "TransitionType == ECameraTransitionType::Custom"))
    UCurveFloat* CustomCurve = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bUseFadeEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects",
        meta = (EditCondition = "bUseFadeEffect"))
    float FadeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects",
        meta = (EditCondition = "bUseFadeEffect"))
    FLinearColor FadeColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bFadeAudio = false;
};

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API UMenuCameraManager : public UObject
{
    GENERATED_BODY()

public:
    UMenuCameraManager();

    // === MAIN INTERFACE ===
    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    void Initialize(APlayerController* InPlayerController);

    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    bool StartTransition(const FString& TargetCameraTag, const FCameraTransitionSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    bool StartTransitionToCamera(ACameraActor* TargetCamera, const FCameraTransitionSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    void StopTransition(bool bSnapToTarget = false);

    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    void ResetToOriginalCamera();

    // === CAMERA MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    ACameraActor* FindCameraByTag(const FString& CameraTag);

    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    void RegisterCamera(const FString& CameraName, ACameraActor* Camera);

    UFUNCTION(BlueprintCallable, Category = "Camera Manager")
    ACameraActor* CreateFallbackCamera(const FString& CameraName, const FVector& Location, const FRotator& Rotation);

    // === STATUS ===
    UFUNCTION(BlueprintPure, Category = "Camera Manager")
    bool IsTransitionActive() const { return bTransitionInProgress; }

    UFUNCTION(BlueprintPure, Category = "Camera Manager")
    float GetTransitionProgress() const { return TransitionAlpha; }

    UFUNCTION(BlueprintPure, Category = "Camera Manager")
    FString GetCurrentTransitionName() const { return CurrentTransitionName; }

    // === EVENTS ===
    UPROPERTY(BlueprintAssignable, Category = "Camera Manager")
    FOnCameraTransitionComplete OnTransitionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Camera Manager")
    FOnCameraTransitionStarted OnTransitionStarted;

    // === PRESET TRANSITIONS ===
    UFUNCTION(BlueprintCallable, Category = "Camera Manager|Presets")
    bool TransitionToLobby(const FCameraTransitionSettings& Settings = FCameraTransitionSettings());

    UFUNCTION(BlueprintCallable, Category = "Camera Manager|Presets")
    bool TransitionToMainMenu(const FCameraTransitionSettings& Settings = FCameraTransitionSettings());

    UFUNCTION(BlueprintCallable, Category = "Camera Manager|Presets")
    bool TransitionToGameplay(const FCameraTransitionSettings& Settings = FCameraTransitionSettings());

    UPROPERTY(EditDefaultsOnly, Category = "CameraType")
    ECameraTransitionType CameraTransitionType;
protected:
    // === CORE TRANSITION LOGIC ===
    void UpdateTransition();
    void CompleteTransition();
    float ApplyEasingFunction(float Alpha, ECameraTransitionType EasingType);
    void StartFadeEffect(bool bFadeIn);

    // === CAMERA UTILITIES ===
    void StoreOriginalCameraState();
    void ApplyCameraTransform(const FVector& Location, const FRotator& Rotation);

private:
    // === CORE COMPONENTS ===
    UPROPERTY()
    APlayerController* PlayerController = nullptr;

    UPROPERTY()
    class APlayerCameraManager* CameraManager = nullptr;

    // === REGISTERED CAMERAS ===
    UPROPERTY()
    TMap<FString, ACameraActor*> RegisteredCameras;

    // === TRANSITION STATE ===
    bool bTransitionInProgress = false;
    float TransitionAlpha = 0.0f;
    FString CurrentTransitionName;

    UPROPERTY()
    ACameraActor* TargetCamera = nullptr;

    FCameraTransitionSettings CurrentSettings;

    // === CAMERA STATES ===
    FVector OriginalCameraLocation;
    FRotator OriginalCameraRotation;
    FVector TargetCameraLocation;
    FRotator TargetCameraRotation;

    // === TIMERS ===
    FTimerHandle TransitionTimer;
    FTimerHandle FadeTimer;

    // === CONSTANTS ===
    static const float TRANSITION_UPDATE_RATE;
    static const FString LOBBY_CAMERA_TAG;
    static const FString MENU_CAMERA_TAG;
    static const FString GAMEPLAY_CAMERA_TAG;

};