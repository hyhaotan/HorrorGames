#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameModeSelection.generated.h"

// Forward declarations
class UButton;
class UWidgetAnimation;
class UMainMenu;
class UServerBrowserWidget;
class ULobbyWidget;

UENUM()
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

    UFUNCTION() void OnShownimationFinished();
    UFUNCTION() void OnHideAnimationFinished();
    UFUNCTION() void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

protected:
    /** UI Elements */
    UPROPERTY(meta = (BindWidget)) UButton* SinglePlayerButton;
    UPROPERTY(meta = (BindWidget)) UButton* MultiplayerButton;
    UPROPERTY(meta = (BindWidget)) UButton* BackButton;
    UPROPERTY(meta = (BindWidgetAnim), Transient) UWidgetAnimation* ShowAnim;
    UPROPERTY(meta = (BindWidgetAnim), Transient) UWidgetAnimation* HideAnim;

    /** Widget classes for navigation */
    UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<UMainMenu> MainMenuClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<ULobbyWidget> LobbyWidgetClass;

private:
    /** Button callbacks */
    UFUNCTION() void OnSinglePlayerClicked();
    UFUNCTION() void OnMultiplayerClicked();
    UFUNCTION() void OnBackClicked();

    /** State enumeration for animation flow */
    ETransitionState PendingState;
    IOnlineSessionPtr SessionInterface;

    void CreateLobbySession();
};