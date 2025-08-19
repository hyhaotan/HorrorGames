#include "HorrorGame/Widget/Menu/MainMenu.h"
#include "HorrorGame/Widget/Menu/GameModeSelection.h"
#include "HorrorGame/Widget/Settings/GraphicsWidget.h"
#include "HorrorGame/Widget/Menu/ConfirmExitWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

void UMainMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize time tracking
    HoveredButtonIndex = -1;

    // === BIND ORIGINAL BUTTONS ===
    if (PlayButton)
    {
        PlayButton->OnClicked.AddDynamic(this, &UMainMenu::OnPlayButtonClicked);
        PlayButton->OnHovered.AddDynamic(this, &UMainMenu::OnPlayButtonHovered);
        PlayButton->OnUnhovered.AddDynamic(this, &UMainMenu::OnPlayButtonUnhovered);
    }

    if (OptionsButton)
    {
        OptionsButton->OnClicked.AddDynamic(this, &UMainMenu::OnOptionsButtonClicked);
        OptionsButton->OnHovered.AddDynamic(this, &UMainMenu::OnOptionsButtonHovered);
        OptionsButton->OnUnhovered.AddDynamic(this, &UMainMenu::OnOptionsButtonUnhovered);
    }

    if (ExitButton)
    {
        ExitButton->OnClicked.AddDynamic(this, &UMainMenu::OnExitButtonClicked);
        ExitButton->OnHovered.AddDynamic(this, &UMainMenu::OnExitButtonHovered);
        ExitButton->OnUnhovered.AddDynamic(this, &UMainMenu::OnExitButtonUnhovered);
    }

    if (CreditsButton)
    {
        CreditsButton->OnClicked.AddDynamic(this, &UMainMenu::OnCreditsButtonClicked);
        CreditsButton->OnHovered.AddDynamic(this, &UMainMenu::OnCreditsButtonHovered);
        CreditsButton->OnUnhovered.AddDynamic(this, &UMainMenu::OnCreditsButtonUnhovered);
    }

    // === PLAY ENTRANCE ANIMATIONS ===
    if (ButtonEntranceAnim)
    {
        // Stagger entrance. Note: ButtonEntranceAnim should be set up in UMG to animate each button
        PlayButtonAnimation(0, 0.2f); // Play
        PlayButtonAnimation(1, 0.4f); // Options
        PlayButtonAnimation(2, 0.6f); // Exit
        PlayButtonAnimation(3, 1.0f); // Credits
    }

    // === START BACKGROUND SYSTEMS ===
    StartAmbienceLoop();

    // Set version text if available
    if (VersionText)
    {
        VersionText->SetText(FText::FromString(TEXT("v1.0.0 - Alpha Build")));
    }
}

void UMainMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMainMenu::StartAmbienceLoop()
{
    if (AmbienceSound)
    {
        UGameplayStatics::PlaySound2D(this, AmbienceSound, 0.6f, 1.0f, 0.0f);

        // Loop ambience every 3 minutes
        FTimerHandle AmbienceTimer;
        GetWorld()->GetTimerManager().SetTimer(AmbienceTimer, [this]()
            {
                if (AmbienceSound)
                {
                    UGameplayStatics::PlaySound2D(this, AmbienceSound, 0.6f, 1.0f, 0.0f);
                }
            }, 180.0f, true);
    }
}

// === ORIGINAL BUTTON IMPLEMENTATIONS ===
void UMainMenu::OnPlayButtonClicked()
{
    PlayUISound(ButtonClickSound);

    if (GameModeSelectionClass)
    {
        GameModeSelection = CreateWidget<UGameModeSelection>(GetWorld(), GameModeSelectionClass);
        if (GameModeSelection)
        {
            GameModeSelection->AddToViewport();
            this->RemoveFromViewport();
        }
    }
}

void UMainMenu::OnOptionsButtonClicked()
{
    PlayUISound(ButtonClickSound);

    if (GraphicsWidgetClass)
    {
        GraphicsWidget = CreateWidget<UGraphicsWidget>(GetWorld(), GraphicsWidgetClass);
        if (GraphicsWidget)
        {
            GraphicsWidget->AddToViewport();
        }
    }
}

void UMainMenu::OnExitButtonClicked()
{
    PlayUISound(ButtonClickSound);

    if (!ConfirmExitWidgetClass) return;
    if (!ConfirmExitWidgetInstance)
    {
        ConfirmExitWidgetInstance = CreateWidget<UConfirmExitWidget>(GetWorld(), ConfirmExitWidgetClass);
        if (ConfirmExitWidgetInstance)
        {
            ConfirmExitWidgetInstance->ShowAnimExit();
            ConfirmExitWidgetInstance->AddToViewport(100);
        }
    }
    else
    {
        ConfirmExitWidgetInstance->ShowAnimExit();
        ConfirmExitWidgetInstance->SetVisibility(ESlateVisibility::Visible);
    }
}

void UMainMenu::OnCreditsButtonClicked()
{
    PlayUISound(ButtonClickSound);
    UE_LOG(LogTemp, Log, TEXT("Credits button clicked"));

    if (CreditsWidgetClass)
    {
        auto CreditsWidget = CreateWidget<UUserWidget>(GetWorld(), CreditsWidgetClass);
        if (CreditsWidget)
        {
            CreditsWidget->AddToViewport(50);
        }
    }
}

// === HOVER EFFECTS ===
void UMainMenu::OnPlayButtonHovered()
{
    PlayUISound(ButtonHoverSound);
    HoveredButtonIndex = 0;
    PlayHoverAnimationForButton(0);
}

void UMainMenu::OnPlayButtonUnhovered()
{
    StopHoverAnimationForButton(0);
    HoveredButtonIndex = -1;
}

void UMainMenu::OnOptionsButtonHovered()
{
    PlayUISound(ButtonHoverSound);
    HoveredButtonIndex = 1;
    PlayHoverAnimationForButton(1);
}

void UMainMenu::OnOptionsButtonUnhovered()
{
    StopHoverAnimationForButton(1);
    HoveredButtonIndex = -1;
}

void UMainMenu::OnExitButtonHovered()
{
    PlayUISound(ButtonHoverSound);
    HoveredButtonIndex = 2;
    PlayHoverAnimationForButton(2);
}

void UMainMenu::OnExitButtonUnhovered()
{
    StopHoverAnimationForButton(2);
    HoveredButtonIndex = -1;
}

void UMainMenu::OnCreditsButtonHovered()
{
    PlayUISound(ButtonHoverSound);
    HoveredButtonIndex = 3;
    PlayHoverAnimationForButton(3);
}

void UMainMenu::OnCreditsButtonUnhovered()
{
    StopHoverAnimationForButton(3);
    HoveredButtonIndex = -1;
}

void UMainMenu::PlayButtonAnimation(int32 ButtonIndex, float Delay)
{
    FTimerHandle AnimTimer;
    GetWorld()->GetTimerManager().SetTimer(AnimTimer, [this, ButtonIndex]()
        {
            if (ButtonEntranceAnim)
            {
                PlayAnimation(ButtonEntranceAnim);
            }
        }, Delay, false);
}

void UMainMenu::PlayUISound(USoundCue* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(this, Sound, 0.8f);
    }
}

void UMainMenu::PlayHoverAnimationForButton(int32 ButtonIndex)
{
    switch (ButtonIndex)
    {
    case 0:
        if (PlayButtonHoverAnim) PlayAnimation(PlayButtonHoverAnim);
        break;
    case 1:
        if (OptionsButtonHoverAnim) PlayAnimation(OptionsButtonHoverAnim);
        break;
    case 2:
        if (ExitButtonHoverAnim) PlayAnimation(ExitButtonHoverAnim);
        break;
    case 3:
        if (CreditsButtonHoverAnim) PlayAnimation(CreditsButtonHoverAnim);
        break;
    default:
        break;
    }
}

void UMainMenu::StopHoverAnimationForButton(int32 ButtonIndex)
{
    switch (ButtonIndex)
    {
    case 0:
        if (PlayButtonHoverAnim) StopAnimation(PlayButtonHoverAnim);
        break;
    case 1:
        if (OptionsButtonHoverAnim) StopAnimation(OptionsButtonHoverAnim);
        break;
    case 2:
        if (ExitButtonHoverAnim) StopAnimation(ExitButtonHoverAnim);
        break;
    case 3:
        if (CreditsButtonHoverAnim) StopAnimation(CreditsButtonHoverAnim);
        break;
    default:
        break;
    }
}
