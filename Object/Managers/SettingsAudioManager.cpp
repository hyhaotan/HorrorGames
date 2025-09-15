#include "SettingsAudioManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

USettingsAudioManager::USettingsAudioManager()
    : TabSwitchSound(nullptr)
    , HoverSound(nullptr)
    , ApplySound(nullptr)
    , ResetSound(nullptr)
    , BackSound(nullptr)
    , ErrorSound(nullptr)
    , SettingsChangedSound(nullptr)
    , MasterVolume(1.0f)
    , bAudioEnabled(true)
    , FadeInAnimation(nullptr)
    , FadeOutAnimation(nullptr)
    , TabTransitionAnimation(nullptr)
    , ButtonHoverAnimation(nullptr)
    , ParentWidget(nullptr)
{
}

void USettingsAudioManager::PlaySound(ESoundTypes SoundType)
{
    if (!bAudioEnabled || MasterVolume <= 0.0f)
    {
        return;
    }

    USoundCue* SoundToPlay = GetSoundCue(SoundType);
    if (SoundToPlay)
    {
        PlaySoundInternal(SoundToPlay);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsAudioManager::PlaySound - No sound cue for type %d"), (int32)SoundType);
    }
}

void USettingsAudioManager::SetSoundCues(USoundCue* TabSwitch, USoundCue* Hover, USoundCue* Apply,
    USoundCue* Reset, USoundCue* Back, USoundCue* Error, USoundCue* SettingsChanged)
{
    TabSwitchSound = TabSwitch;
    HoverSound = Hover;
    ApplySound = Apply;
    ResetSound = Reset;
    BackSound = Back;
    ErrorSound = Error;
    SettingsChangedSound = SettingsChanged;

    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: All sound cues set"));
}

void USettingsAudioManager::SetSoundCue(ESoundTypes SoundType, USoundCue* SoundCue)
{
    switch (SoundType)
    {
    case ESoundTypes::TabSwitch:
        TabSwitchSound = SoundCue;
        break;
    case ESoundTypes::Hover:
        HoverSound = SoundCue;
        break;
    case ESoundTypes::Apply:
        ApplySound = SoundCue;
        break;
    case ESoundTypes::Reset:
        ResetSound = SoundCue;
        break;
    case ESoundTypes::Back:
        BackSound = SoundCue;
        break;
    case ESoundTypes::Error:
        ErrorSound = SoundCue;
        break;
    case ESoundTypes::SettingsChanged:
        SettingsChangedSound = SoundCue;
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("SettingsAudioManager::SetSoundCue - Invalid sound type"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Sound cue set for type %d"), (int32)SoundType);
}

void USettingsAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Master volume set to %f"), MasterVolume);
}

void USettingsAudioManager::SetAudioEnabled(bool bEnabled)
{
    bAudioEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Audio %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void USettingsAudioManager::PlayTabTransition()
{
    if (ParentWidget.IsValid() && TabTransitionAnimation)
    {
        ParentWidget->PlayAnimation(TabTransitionAnimation);
        UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Playing tab transition animation"));
    }
}

void USettingsAudioManager::PlayFadeIn()
{
    if (ParentWidget.IsValid() && FadeInAnimation)
    {
        ParentWidget->PlayAnimation(FadeInAnimation);
        UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Playing fade in animation"));
    }
}

void USettingsAudioManager::PlayFadeOut()
{
    if (ParentWidget.IsValid() && FadeOutAnimation)
    {
        // Bind to animation finished event for callback
        if (UWorld* World = ParentWidget->GetWorld())
        {
            FTimerHandle TimerHandle;
            float AnimationLength = FadeOutAnimation->GetEndTime();

            World->GetTimerManager().SetTimer(TimerHandle, this, &USettingsAudioManager::OnFadeOutAnimationFinished, AnimationLength, false);
        }

        ParentWidget->PlayAnimationReverse(FadeOutAnimation);
        UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Playing fade out animation"));
    }
    else
    {
        // If no animation, immediately call the callback
        OnFadeOutAnimationFinished();
    }
}

void USettingsAudioManager::PlayButtonHover()
{
    if (ParentWidget.IsValid() && ButtonHoverAnimation)
    {
        ParentWidget->PlayAnimation(ButtonHoverAnimation);
    }
}

void USettingsAudioManager::SetAnimations(UWidgetAnimation* FadeIn, UWidgetAnimation* FadeOut,
    UWidgetAnimation* TabTransition, UWidgetAnimation* ButtonHover)
{
    FadeInAnimation = FadeIn;
    FadeOutAnimation = FadeOut;
    TabTransitionAnimation = TabTransition;
    ButtonHoverAnimation = ButtonHover;

    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: All animations set"));
}

void USettingsAudioManager::SetParentWidget(UUserWidget* InParentWidget)
{
    ParentWidget = InParentWidget;
    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Parent widget set"));
}

USoundCue* USettingsAudioManager::GetSoundCue(ESoundTypes SoundType) const
{
    switch (SoundType)
    {
    case ESoundTypes::TabSwitch: return TabSwitchSound;
    case ESoundTypes::Hover: return HoverSound;
    case ESoundTypes::Apply: return ApplySound;
    case ESoundTypes::Reset: return ResetSound;
    case ESoundTypes::Back: return BackSound;
    case ESoundTypes::Error: return ErrorSound;
    case ESoundTypes::SettingsChanged: return SettingsChangedSound;
    default: return nullptr;
    }
}

void USettingsAudioManager::PlaySoundInternal(USoundCue* SoundCue)
{
    if (!SoundCue || !ParentWidget.IsValid())
    {
        return;
    }

    UWorld* World = ParentWidget->GetWorld();
    if (World)
    {
        UGameplayStatics::PlaySound2D(World, SoundCue, MasterVolume);
    }
}

void USettingsAudioManager::OnFadeOutAnimationFinished()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Fade out animation completed"));
    OnFadeOutCompleted.Broadcast();
}

void USettingsAudioManager::DebugTestAllSounds()
{
    UE_LOG(LogTemp, Log, TEXT("SettingsAudioManager: Testing all sounds..."));

    TArray<ESoundTypes> SoundTypes = {
        ESoundTypes::TabSwitch,
        ESoundTypes::Hover,
        ESoundTypes::Apply,
        ESoundTypes::Reset,
        ESoundTypes::Back,
        ESoundTypes::Error,
        ESoundTypes::SettingsChanged
    };

    for (ESoundTypes SoundType : SoundTypes)
    {
        PlaySound(SoundType);
        // Small delay between sounds for testing
        if (UWorld* World = GetWorld())
        {
            FTimerHandle DelayHandle;
            World->GetTimerManager().SetTimer(DelayHandle, [] {}, 0.5f, false);
        }
    }
}

void USettingsAudioManager::DebugPrintAudioConfig() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SettingsAudioManager Debug Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Audio Enabled: %s"), bAudioEnabled ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("Master Volume: %f"), MasterVolume);
    UE_LOG(LogTemp, Log, TEXT("TabSwitchSound: %s"), TabSwitchSound ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("HoverSound: %s"), HoverSound ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("ApplySound: %s"), ApplySound ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("ResetSound: %s"), ResetSound ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("BackSound: %s"), BackSound ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("ErrorSound: %s"), ErrorSound ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("SettingsChangedSound: %s"), SettingsChangedSound ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT("Parent Widget: %s"), ParentWidget.IsValid() ? TEXT("Valid") : TEXT("Invalid"));
    UE_LOG(LogTemp, Log, TEXT("=== End Debug Info ==="));
}