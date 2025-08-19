#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Animation/WidgetAnimation.h"
#include "Sound/SoundCue.h"
#include "Engine/Texture2D.h"
#include "MainMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuTransition);

UCLASS()
class HORRORGAME_API UMainMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // === ORIGINAL BUTTONS ===
    UPROPERTY(meta = (BindWidget))
    class UButton* PlayButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* OptionsButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* ExitButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* CreditsButton;

    // === UI ELEMENTS ===

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VersionText;

    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* ParticleContainer;

    // === ANIMATIONS ===
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* ButtonEntranceAnim;

    // Hover animations - each should only animate the corresponding button in UMG
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* PlayButtonHoverAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* OptionsButtonHoverAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* ExitButtonHoverAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* CreditsButtonHoverAnim;

    // === SOUNDS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ButtonHoverSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ButtonClickSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ThunderSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* CreepyWhisperSound;

    // === WIDGET CLASSES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UGameModeSelection> GameModeSelectionClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UGraphicsWidget> GraphicsWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UConfirmExitWidget> ConfirmExitWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> CreditsWidgetClass;

private:
    // === WIDGET INSTANCES ===
    UPROPERTY()
    class UGameModeSelection* GameModeSelection;

    UPROPERTY()
    class UGraphicsWidget* GraphicsWidget;

    UPROPERTY()
    class UConfirmExitWidget* ConfirmExitWidgetInstance;

    // === PRIVATE VARIABLES ===
    int32 HoveredButtonIndex;

    // === BUTTON EVENT HANDLERS ===
    UFUNCTION()
    void OnPlayButtonClicked();

    UFUNCTION()
    void OnOptionsButtonClicked();

    UFUNCTION()
    void OnExitButtonClicked();

    UFUNCTION()
    void OnCreditsButtonClicked();

    // === BUTTON HOVER HANDLERS ===
    UFUNCTION()
    void OnPlayButtonHovered();

    UFUNCTION()
    void OnPlayButtonUnhovered();

    UFUNCTION()
    void OnOptionsButtonHovered();

    UFUNCTION()
    void OnOptionsButtonUnhovered();

    UFUNCTION()
    void OnExitButtonHovered();

    UFUNCTION()
    void OnExitButtonUnhovered();

    UFUNCTION()
    void OnCreditsButtonHovered();

    UFUNCTION()
    void OnCreditsButtonUnhovered();

    // === UTILITY FUNCTIONS ===
    void PlayButtonAnimation(int32 ButtonIndex, float Delay);
    void PlayUISound(USoundCue* Sound);
    void StartAmbienceLoop();

    void PlayHoverAnimationForButton(int32 ButtonIndex);
    void StopHoverAnimationForButton(int32 ButtonIndex);

public:
    // === DELEGATES ===
    UPROPERTY(BlueprintAssignable)
    FOnMenuTransition OnMenuTransition;
};
