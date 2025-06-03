#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameModeSelection.generated.h"

class UButton;
class ULobbyWidget;
class UMainMenu;

UENUM(BlueprintType)
enum class EGameModeState : uint8
{
    None        UMETA(DisplayName = "None"),
    Single      UMETA(DisplayName = "SinglePlayer"),
    Multiplayer UMETA(DisplayName = "Multiplayer"),
    Back        UMETA(DisplayName = "Back")
};

UCLASS()
class HORRORGAME_API UGameModeSelection : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode")
    EGameModeState CurrentState = EGameModeState::None;

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* SinglePlayerButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MultiplayerButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* ShowAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* HideAnim;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<ULobbyWidget> LobbyWidgetClass;

    UPROPERTY(EditAnywhere, Category = "Classes")
    TSubclassOf<UMainMenu> MainMenuClass;

private:
    const FName SessionName = NAME_GameSession;

    UFUNCTION()
    void OnSinglePlayerButtonClicked();

    UFUNCTION()
    void OnMultiplayerButtonClicked();

    UFUNCTION()
    void OnBackButtonClicked();

    UFUNCTION()
    void OnHideAnimationFinished();

    void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);

    void StartSinglePlayer();
    void StartMultiplayer();
};