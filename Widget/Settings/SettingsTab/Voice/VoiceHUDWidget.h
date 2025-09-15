#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "HorrorGame/GameInstance/VoiceChatManager.h"
#include "VoiceHUDWidget.generated.h"

USTRUCT(BlueprintType)
struct FVoicePlayerHUDInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(BlueprintReadWrite)
    bool bIsTalking;

    UPROPERTY(BlueprintReadWrite)
    bool bIsMuted;

    UPROPERTY(BlueprintReadWrite)
    float VoiceLevel;

    UPROPERTY(BlueprintReadWrite)
    float Distance;

    FVoicePlayerHUDInfo()
    {
        PlayerName = TEXT("");
        bIsTalking = false;
        bIsMuted = false;
        VoiceLevel = 0.0f;
        Distance = 0.0f;
    }
};

UCLASS()
class HORRORGAME_API UVoiceHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UVoiceHUDWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // UI Updates
    UFUNCTION(BlueprintCallable, Category = "Voice HUD")
    void UpdatePlayersList();

    UFUNCTION(BlueprintCallable, Category = "Voice HUD")
    void UpdateMicrophoneStatus();

    UFUNCTION(BlueprintCallable, Category = "Voice HUD")
    void UpdateVoiceActivity();

    UFUNCTION(BlueprintCallable, Category = "Voice HUD")
    void ShowVoiceIndicator(bool bShow);

    UFUNCTION(BlueprintCallable, Category = "Voice HUD")
    void SetHorrorMode(bool bEnabled);

    // Player-specific UI
    UFUNCTION(BlueprintImplementableEvent, Category = "Voice HUD")
    class UUserWidget* CreatePlayerVoiceWidget(const FVoicePlayerHUDInfo& PlayerInfo);

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice HUD")
    void UpdatePlayerVoiceWidget(class UUserWidget* Widget, const FVoicePlayerHUDInfo& PlayerInfo);

protected:
    // Main UI Components
    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* PlayersListBox;

    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* MicrophoneStatusBox;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MicrophoneStatusText;

    UPROPERTY(meta = (BindWidget))
    class UImage* MicrophoneStatusIcon;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* VoiceActivityBar;

    UPROPERTY(meta = (BindWidget))
    class UButton* MuteToggleButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* PushToTalkHintText;

    // Horror-specific UI
    UPROPERTY(meta = (BindWidget))
    class UImage* StaticNoiseOverlay;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VoiceDistortionIndicator;

    UPROPERTY(meta = (BindWidget))
    class UImage* VoiceCutoutWarning;

    // Proximity Voice UI
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ProximityRangeText;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* ProximityIndicator;

private:
    // Event handlers
    UFUNCTION()
    void OnPlayerStartedTalking(const FString& PlayerId);

    UFUNCTION()
    void OnPlayerStoppedTalking(const FString& PlayerId);

    UFUNCTION()
    void OnPlayerJoinedVoiceChat(const FString& PlayerId);

    UFUNCTION()
    void OnPlayerLeftVoiceChat(const FString& PlayerId);

    UFUNCTION()
    void OnVoiceChatStatusChanged(bool bIsConnected);

    UFUNCTION()
    void OnMuteToggleClicked();

    // UI Management
    void BindToVoiceChatEvents();
    void UnbindFromVoiceChatEvents();
    void UpdateHorrorEffects();
    void AnimateVoiceIndicator();
    void UpdateProximityIndicator();

    // Internal state
    UPROPERTY()
    UVoiceChatManager* VoiceChatManager;

    UPROPERTY()
    TMap<FString, UUserWidget*> PlayerWidgets;

    bool bHorrorModeEnabled;
    bool bVoiceIndicatorVisible;
    float VoiceIndicatorAnimTime;
    float HorrorEffectIntensity;

    // Animation properties
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* VoiceIndicatorPulse;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* StaticNoiseFlicker;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* VoiceCutoutFlash;
};