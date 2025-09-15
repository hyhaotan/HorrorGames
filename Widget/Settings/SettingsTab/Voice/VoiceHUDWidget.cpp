#include "VoiceHUDWidget.h"
#include "HorrorGame/GameInstance/VoiceChatManager.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include "Components/Button.h"
#include "Engine/Engine.h"
#include "Animation/WidgetAnimation.h"

UVoiceHUDWidget::UVoiceHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bHorrorModeEnabled = false;
    bVoiceIndicatorVisible = false;
    VoiceIndicatorAnimTime = 0.0f;
    HorrorEffectIntensity = 0.0f;
    VoiceChatManager = nullptr;
}

void UVoiceHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get voice chat manager
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        VoiceChatManager = GameInstance->GetSubsystem<UVoiceChatManager>();
    }

    // Bind events
    BindToVoiceChatEvents();

    // Bind button events
    if (MuteToggleButton)
    {
        MuteToggleButton->OnClicked.AddDynamic(this, &UVoiceHUDWidget::OnMuteToggleClicked);
    }

    // Initial UI setup
    UpdateMicrophoneStatus();
    UpdatePlayersList();
    ShowVoiceIndicator(false);

    // Initialize horror mode based on game settings
    if (UHorrorGameSettings* Settings = UHorrorGameSettings::Get())
    {
        SetHorrorMode(true); // Always enable for horror game
    }
}

void UVoiceHUDWidget::NativeDestruct()
{
    UnbindFromVoiceChatEvents();

    // Clear player widgets
    for (auto& Pair : PlayerWidgets)
    {
        if (Pair.Value)
        {
            Pair.Value->RemoveFromParent();
        }
    }
    PlayerWidgets.Empty();

    Super::NativeDestruct();
}

void UVoiceHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateVoiceActivity();
    UpdateProximityIndicator();

    if (bHorrorModeEnabled)
    {
        UpdateHorrorEffects();
    }

    // Update voice indicator animation
    if (bVoiceIndicatorVisible)
    {
        AnimateVoiceIndicator();
    }
}

void UVoiceHUDWidget::UpdatePlayersList()
{
    if (!VoiceChatManager || !PlayersListBox)
    {
        return;
    }

    TArray<FVoiceChatPlayer> Players = VoiceChatManager->GetVoiceChatPlayers();

    // Update existing widgets and create new ones
    TArray<FString> CurrentPlayerIds;

    for (const FVoiceChatPlayer& Player : Players)
    {
        CurrentPlayerIds.Add(Player.PlayerId);

        UUserWidget** FoundWidget = PlayerWidgets.Find(Player.PlayerId);
        UUserWidget* PlayerWidget = nullptr;

        if (FoundWidget && *FoundWidget)
        {
            PlayerWidget = *FoundWidget;
        }
        else
        {
            // Create new player widget
            FVoicePlayerHUDInfo HUDInfo;
            HUDInfo.PlayerName = Player.PlayerName;
            HUDInfo.bIsTalking = Player.bIsTalking;
            HUDInfo.bIsMuted = Player.bIsMuted;
            HUDInfo.VoiceLevel = Player.VoiceLevel;
            HUDInfo.Distance = Player.DistanceFromLocalPlayer;

            PlayerWidget = CreatePlayerVoiceWidget(HUDInfo);

            if (PlayerWidget)
            {
                PlayerWidgets.Add(Player.PlayerId, PlayerWidget);
                PlayersListBox->AddChild(PlayerWidget);
            }
        }

        // Update existing widget
        if (PlayerWidget)
        {
            FVoicePlayerHUDInfo HUDInfo;
            HUDInfo.PlayerName = Player.PlayerName;
            HUDInfo.bIsTalking = Player.bIsTalking;
            HUDInfo.bIsMuted = Player.bIsMuted;
            HUDInfo.VoiceLevel = Player.VoiceLevel;
            HUDInfo.Distance = Player.DistanceFromLocalPlayer;

            UpdatePlayerVoiceWidget(PlayerWidget, HUDInfo);
        }
    }

    // Remove widgets for players who left
    TArray<FString> PlayersToRemove;
    for (auto& Pair : PlayerWidgets)
    {
        if (!CurrentPlayerIds.Contains(Pair.Key))
        {
            PlayersToRemove.Add(Pair.Key);
        }
    }

    for (const FString& PlayerId : PlayersToRemove)
    {
        if (UUserWidget** FoundWidget = PlayerWidgets.Find(PlayerId))
        {
            if (*FoundWidget)
            {
                (*FoundWidget)->RemoveFromParent();
            }
        }
        PlayerWidgets.Remove(PlayerId);
    }
}

void UVoiceHUDWidget::UpdateMicrophoneStatus()
{
    if (!VoiceChatManager)
    {
        return;
    }

    FString StatusText;
    FLinearColor StatusColor = FLinearColor::White;

    if (!VoiceChatManager->IsVoiceChatConnected())
    {
        StatusText = TEXT("Voice Chat Disconnected");
        StatusColor = FLinearColor::Red;
    }
    else if (VoiceChatManager->IsMicrophoneMuted())
    {
        StatusText = TEXT("Muted");
        StatusColor = FLinearColor::Red;
    }
    else if (VoiceChatManager->IsRecording())
    {
        StatusText = TEXT("Recording");
        StatusColor = FLinearColor::Green;
        ShowVoiceIndicator(true);
    }
    else
    {
        StatusText = TEXT("Standby");
        StatusColor = FLinearColor::Yellow;
        ShowVoiceIndicator(false);
    }

    if (MicrophoneStatusText)
    {
        MicrophoneStatusText->SetText(FText::FromString(StatusText));
        MicrophoneStatusText->SetColorAndOpacity(FSlateColor(StatusColor));
    }

    if (MicrophoneStatusIcon)
    {
        MicrophoneStatusIcon->SetColorAndOpacity(StatusColor);
    }

    // Update push-to-talk hint
    if (PushToTalkHintText)
    {
        if (UHorrorGameSettings* Settings = UHorrorGameSettings::Get())
        {
            if (Settings->GetVoiceType() == 0) // Push to talk
            {
                FString KeyName = Settings->GetPushToTalkKey().ToString();
                FString HintText = FString::Printf(TEXT("Hold %s to talk"), *KeyName);
                PushToTalkHintText->SetText(FText::FromString(HintText));
                PushToTalkHintText->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                PushToTalkHintText->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
}

void UVoiceHUDWidget::UpdateVoiceActivity()
{
    if (!VoiceChatManager || !VoiceActivityBar)
    {
        return;
    }

    float VoiceLevel = VoiceChatManager->GetCurrentVoiceLevel();
    VoiceActivityBar->SetPercent(VoiceLevel);

    // Color based on voice level
    FLinearColor BarColor = FLinearColor::Green;
    if (VoiceLevel > 0.8f)
    {
        BarColor = FLinearColor::Red;
    }
    else if (VoiceLevel > 0.5f)
    {
        BarColor = FLinearColor::Yellow;
    }

    VoiceActivityBar->SetFillColorAndOpacity(BarColor);
}

void UVoiceHUDWidget::ShowVoiceIndicator(bool bShow)
{
    if (bVoiceIndicatorVisible == bShow)
    {
        return;
    }

    bVoiceIndicatorVisible = bShow;

    if (bShow && VoiceIndicatorPulse)
    {
        PlayAnimation(VoiceIndicatorPulse, 0.0f, 0, EUMGSequencePlayMode::PingPong);
    }
    else if (!bShow && VoiceIndicatorPulse)
    {
        StopAnimation(VoiceIndicatorPulse);
    }
}

void UVoiceHUDWidget::SetHorrorMode(bool bEnabled)
{
    bHorrorModeEnabled = bEnabled;

    if (bEnabled)
    {
        // Enable horror-specific UI elements
        if (StaticNoiseOverlay)
        {
            StaticNoiseOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
        }

        if (VoiceDistortionIndicator)
        {
            VoiceDistortionIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }
    else
    {
        // Disable horror effects
        if (StaticNoiseOverlay)
        {
            StaticNoiseOverlay->SetVisibility(ESlateVisibility::Collapsed);
        }

        if (VoiceDistortionIndicator)
        {
            VoiceDistortionIndicator->SetVisibility(ESlateVisibility::Collapsed);
        }

        if (VoiceCutoutWarning)
        {
            VoiceCutoutWarning->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

// Event Handlers
void UVoiceHUDWidget::OnPlayerStartedTalking(const FString& PlayerId)
{
    UE_LOG(LogTemp, Log, TEXT("Player started talking: %s"), *PlayerId);
    UpdatePlayersList();
}

void UVoiceHUDWidget::OnPlayerStoppedTalking(const FString& PlayerId)
{
    UE_LOG(LogTemp, Log, TEXT("Player stopped talking: %s"), *PlayerId);
    UpdatePlayersList();
}

void UVoiceHUDWidget::OnPlayerJoinedVoiceChat(const FString& PlayerId)
{
    UE_LOG(LogTemp, Log, TEXT("Player joined voice chat: %s"), *PlayerId);
    UpdatePlayersList();
}

void UVoiceHUDWidget::OnPlayerLeftVoiceChat(const FString& PlayerId)
{
    UE_LOG(LogTemp, Log, TEXT("Player left voice chat: %s"), *PlayerId);
    UpdatePlayersList();
}

void UVoiceHUDWidget::OnVoiceChatStatusChanged(bool bIsConnected)
{
    UE_LOG(LogTemp, Log, TEXT("Voice chat status changed: %s"), bIsConnected ? TEXT("Connected") : TEXT("Disconnected"));
    UpdateMicrophoneStatus();
}

void UVoiceHUDWidget::OnMuteToggleClicked()
{
    if (!VoiceChatManager)
    {
        return;
    }

    bool bCurrentlyMuted = VoiceChatManager->IsMicrophoneMuted();
    VoiceChatManager->SetMicrophoneMuted(!bCurrentlyMuted);

    UpdateMicrophoneStatus();
}

// Private Methods
void UVoiceHUDWidget::BindToVoiceChatEvents()
{
    if (!VoiceChatManager)
    {
        return;
    }

    VoiceChatManager->OnPlayerStartedTalking.AddDynamic(this, &UVoiceHUDWidget::OnPlayerStartedTalking);
    VoiceChatManager->OnPlayerStoppedTalking.AddDynamic(this, &UVoiceHUDWidget::OnPlayerStoppedTalking);
    VoiceChatManager->OnPlayerJoinedVoiceChat.AddDynamic(this, &UVoiceHUDWidget::OnPlayerJoinedVoiceChat);
    VoiceChatManager->OnPlayerLeftVoiceChat.AddDynamic(this, &UVoiceHUDWidget::OnPlayerLeftVoiceChat);
    VoiceChatManager->OnVoiceChatStatusChanged.AddDynamic(this, &UVoiceHUDWidget::OnVoiceChatStatusChanged);
}

void UVoiceHUDWidget::UnbindFromVoiceChatEvents()
{
    if (!VoiceChatManager)
    {
        return;
    }

    VoiceChatManager->OnPlayerStartedTalking.RemoveDynamic(this, &UVoiceHUDWidget::OnPlayerStartedTalking);
    VoiceChatManager->OnPlayerStoppedTalking.RemoveDynamic(this, &UVoiceHUDWidget::OnPlayerStoppedTalking);
    VoiceChatManager->OnPlayerJoinedVoiceChat.RemoveDynamic(this, &UVoiceHUDWidget::OnPlayerJoinedVoiceChat);
    VoiceChatManager->OnPlayerLeftVoiceChat.RemoveDynamic(this, &UVoiceHUDWidget::OnPlayerLeftVoiceChat);
    VoiceChatManager->OnVoiceChatStatusChanged.RemoveDynamic(this, &UVoiceHUDWidget::OnVoiceChatStatusChanged);
}

void UVoiceHUDWidget::UpdateHorrorEffects()
{
    if (!bHorrorModeEnabled || !VoiceChatManager)
    {
        return;
    }

    // Update static noise overlay based on voice chat activity
    if (StaticNoiseOverlay)
    {
        float StaticIntensity = 0.0f;

        // Add static when there's voice activity or during horror events
        if (VoiceChatManager->IsRecording() || HorrorEffectIntensity > 0.0f)
        {
            StaticIntensity = FMath::Max(0.1f, HorrorEffectIntensity * 0.5f);
            StaticIntensity += FMath::RandRange(-0.05f, 0.05f); // Random flicker
        }

        StaticNoiseOverlay->SetOpacity(StaticIntensity);

        // Play static noise animation if intensity is high
        if (StaticIntensity > 0.3f && StaticNoiseFlicker)
        {
            if (!IsAnimationPlaying(StaticNoiseFlicker))
            {
                PlayAnimation(StaticNoiseFlicker, 0.0f, 0, EUMGSequencePlayMode::Forward);
            }
        }
    }

    // Update voice distortion indicator
    if (VoiceDistortionIndicator)
    {
        if (HorrorEffectIntensity > 0.2f)
        {
            VoiceDistortionIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);

            FString DistortionText;
            if (HorrorEffectIntensity > 0.8f)
            {
                DistortionText = TEXT("SEVERE INTERFERENCE");
            }
            else if (HorrorEffectIntensity > 0.5f)
            {
                DistortionText = TEXT("Signal Distortion");
            }
            else
            {
                DistortionText = TEXT("Interference");
            }

            VoiceDistortionIndicator->SetText(FText::FromString(DistortionText));
            VoiceDistortionIndicator->SetOpacity(HorrorEffectIntensity);
        }
        else
        {
            VoiceDistortionIndicator->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Update voice cutout warning
    if (VoiceCutoutWarning)
    {
        // Show warning when horror effects are very high
        if (HorrorEffectIntensity > 0.9f)
        {
            VoiceCutoutWarning->SetVisibility(ESlateVisibility::HitTestInvisible);

            if (VoiceCutoutFlash && !IsAnimationPlaying(VoiceCutoutFlash))
            {
                PlayAnimation(VoiceCutoutFlash, 0.0f, 0, EUMGSequencePlayMode::PingPong);
            }
        }
        else
        {
            VoiceCutoutWarning->SetVisibility(ESlateVisibility::Collapsed);

            if (VoiceCutoutFlash && IsAnimationPlaying(VoiceCutoutFlash))
            {
                StopAnimation(VoiceCutoutFlash);
            }
        }
    }

    // Gradually decay horror effect intensity
    HorrorEffectIntensity = FMath::Max(0.0f, HorrorEffectIntensity - GetWorld()->GetDeltaSeconds() * 0.2f);
}

void UVoiceHUDWidget::AnimateVoiceIndicator()
{
    if (!bVoiceIndicatorVisible)
    {
        return;
    }

    VoiceIndicatorAnimTime += GetWorld()->GetDeltaSeconds();

    // Create pulsing effect for voice indicator
    float PulseValue = FMath::Sin(VoiceIndicatorAnimTime * 4.0f) * 0.5f + 0.5f;

    if (MicrophoneStatusIcon)
    {
        float Scale = 1.0f + PulseValue * 0.2f; // Scale between 1.0 and 1.2
        MicrophoneStatusIcon->SetRenderScale(FVector2D(Scale, Scale));
    }
}

void UVoiceHUDWidget::UpdateProximityIndicator()
{
    if (!VoiceChatManager || !ProximityIndicator)
    {
        return;
    }

    if (!VoiceChatManager->IsProximityVoiceEnabled())
    {
        ProximityIndicator->SetVisibility(ESlateVisibility::Collapsed);
        if (ProximityRangeText)
        {
            ProximityRangeText->SetVisibility(ESlateVisibility::Collapsed);
        }
        return;
    }

    ProximityIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
    if (ProximityRangeText)
    {
        ProximityRangeText->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    float ProximityRange = VoiceChatManager->GetProximityRange();

    // Find closest talking player
    float ClosestDistance = ProximityRange;
    bool bAnyPlayerInRange = false;

    TArray<FVoiceChatPlayer> Players = VoiceChatManager->GetVoiceChatPlayers();
    for (const FVoiceChatPlayer& Player : Players)
    {
        if (Player.bIsTalking && Player.DistanceFromLocalPlayer < ProximityRange)
        {
            ClosestDistance = FMath::Min(ClosestDistance, Player.DistanceFromLocalPlayer);
            bAnyPlayerInRange = true;
        }
    }

    // Update proximity indicator
    if (bAnyPlayerInRange)
    {
        float ProximityPercent = 1.0f - (ClosestDistance / ProximityRange);
        ProximityIndicator->SetPercent(ProximityPercent);

        // Color based on proximity
        FLinearColor ProximityColor = FLinearColor::Green;
        if (ProximityPercent < 0.3f)
        {
            ProximityColor = FLinearColor::Red;
        }
        else if (ProximityPercent < 0.7f)
        {
            ProximityColor = FLinearColor::Yellow;
        }

        ProximityIndicator->SetFillColorAndOpacity(ProximityColor);
    }
    else
    {
        ProximityIndicator->SetPercent(0.0f);
        ProximityIndicator->SetFillColorAndOpacity(FLinearColor::Gray);
    }

    // Update proximity range text
    if (ProximityRangeText)
    {
        FString RangeText = FString::Printf(TEXT("Range: %.0fm"), ProximityRange / 100.0f);
        ProximityRangeText->SetText(FText::FromString(RangeText));
    }
}