// GameModeSelection.h - Updated version
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameModeSelection.generated.h"

class UMainMenu;
class ULobbyWidget;
class ULobbySessionManager;

// Enum for tracking transition states
UENUM(BlueprintType)
enum class ETransitionState : uint8
{
    None        UMETA(DisplayName = "None"),
    Single      UMETA(DisplayName = "Single Player"),
    Multiplayer UMETA(DisplayName = "Multiplayer"),
    Back        UMETA(DisplayName = "Back to Main Menu")
};

UCLASS()
class HORRORGAME_API UGameModeSelection : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // UI Components
    UPROPERTY(meta = (BindWidget))
    UButton* SinglePlayerButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MultiplayerButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    // Animations
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowAnim;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* HideAnim;

    // Widget Classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UMainMenu> MainMenuClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<ULobbyWidget> LobbyWidgetClass;

private:
    // Button event handlers
    UFUNCTION()
    void OnSinglePlayerClicked();

    UFUNCTION()
    void OnMultiplayerClicked();

    UFUNCTION()
    void OnBackClicked();

    // Animation event handlers
    UFUNCTION()
    void OnHideAnimationFinished();

    UFUNCTION()
    void OnShowAnimationFinished();

    // Session event handlers
    UFUNCTION()
    void OnSessionCreated(bool bWasSuccessful);

    // Utility functions
    void CreateMultiplayerLobby();
    void ReturnToMainMenu();
    void ShowConnectionError();

    // Session Manager
    UPROPERTY()
    ULobbySessionManager* SessionManager;

    // Current transition state
    ETransitionState PendingState = ETransitionState::None;
};