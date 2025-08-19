#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameModeSelection.generated.h"

class UButton;
class UWidgetAnimation;
class UMenuCameraManager;
class ULobbySessionManager;
class ULobbyWidget;
class UMainMenu;
class UCurveFloat;
struct FCameraTransitionSettings;

UENUM(BlueprintType)
enum class ETransitionState : uint8
{
    None,
    Single,
    Multiplayer,
    Back
};

UCLASS()
class HORRORGAME_API UGameModeSelection : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // --- Widgets & Animations (bind these in UMG or set in BP) ---
    UPROPERTY(meta = (BindWidgetOptional))
    UButton* SinglePlayerButton;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* MultiplayerButton;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* BackButton;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* ShowAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* HideAnim;

    // --- Classes to create ---
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<ULobbyWidget> LobbyWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UMainMenu> MainMenuClass;

    // --- Camera transition settings ---
    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraTransitionDuration = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    bool bUseCameraFade = true;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraFadeDuration = 0.4f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    FLinearColor CameraFadeColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, Category = "Camera")
    bool bFadeAudioDuringTransition = true;

    UPROPERTY(EditAnywhere, Category = "Camera")
    UCurveFloat* CameraTransitionCurve = nullptr;

    UPROPERTY(EditAnywhere, Category = "Lobby")
    bool bReturnToMenuOnError = true;

    // --- Runtime objects ---
    UPROPERTY()
    UMenuCameraManager* CameraManager;

    UPROPERTY()
    ULobbySessionManager* SessionManager;

    // internal state
    ETransitionState PendingState = ETransitionState::None;

    // --- Helpers ---
    void BindButtonEvents();
    void InitializeCameraManager();
    void SetButtonsEnabled(bool bEnabled);

    // --- Button callbacks ---
    UFUNCTION()
    void OnSinglePlayerClicked();

    UFUNCTION()
    void OnMultiplayerClicked();

    UFUNCTION()
    void OnBackClicked();

    // --- Camera callbacks ---
    UFUNCTION()
    void OnCameraTransitionStarted(const FString TransitionName);

    UFUNCTION()
    void OnCameraTransitionComplete(bool bWasSuccessful);

    // --- Session callbacks ---
    UFUNCTION()
    void OnSessionCreated(bool bWasSuccessful);

    // --- Anim callback ---
    UFUNCTION()
    void OnHideAnimationFinished();

    UFUNCTION()
    void OnShowAnimationFinished();

    // --- Flow helpers ---
    void StartCameraTransitionToLobby();
    void CreateMultiplayerLobby();
    void ShowLobbyUIDirectly();
    void ShowConnectionError();
    void ReturnToMainMenu();
};
