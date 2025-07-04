#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameModeSelection.generated.h"

// Forward declarations
class UButton;
class UWidgetAnimation;
class UMainMenu;
class UServerBrowserWidget;

UCLASS()
class HORRORGAME_API UGameModeSelection : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION() void OnShownimationFinished();
    UFUNCTION() void OnHideAnimationFinished();

protected:
    /** UI Elements */
    UPROPERTY(meta = (BindWidget)) UButton* SinglePlayerButton;
    UPROPERTY(meta = (BindWidget)) UButton* MultiplayerButton;
    UPROPERTY(meta = (BindWidget)) UButton* BackButton;
    UPROPERTY(meta = (BindWidgetAnim), Transient) UWidgetAnimation* ShowAnim;
    UPROPERTY(meta = (BindWidgetAnim), Transient) UWidgetAnimation* HideAnim;

    /** Widget classes for navigation */
    UPROPERTY(EditAnywhere, Category = "Widgets") TSubclassOf<UMainMenu> MainMenuClass;
    UPROPERTY(EditAnywhere, Category = "Widgets") TSubclassOf<UServerBrowserWidget> ServerBrowserWidgetClass;

private:
    /** Button callbacks */
    UFUNCTION() void OnSinglePlayerClicked();
    UFUNCTION() void OnMultiplayerClicked();
    UFUNCTION() void OnBackClicked();

    /** State enumeration for animation flow */
    enum class ETransitionState : uint8 { None, Single, Multiplayer, Back };
    ETransitionState PendingState = ETransitionState::None;
};