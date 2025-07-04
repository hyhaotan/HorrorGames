#include "GameModeSelection.h"
#include "HorrorGame/Widget/Menu/MainMenu.h"
#include "HorrorGame/Widget/SessionBrowser/ServerBrowserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"

void UGameModeSelection::NativeConstruct()
{
    Super::NativeConstruct();

    if (ShowAnim)
    {
        PlayAnimation(ShowAnim);
    }

    if (SinglePlayerButton)
    {
        SinglePlayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnSinglePlayerClicked);
    }
    if (MultiplayerButton)
    {
        MultiplayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnMultiplayerClicked);
    }
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnBackClicked);
    }

    if (HideAnim)
    {
        FWidgetAnimationDynamicEvent AnimationFinished;
        AnimationFinished.BindDynamic(this, &UGameModeSelection::OnHideAnimationFinished);
        BindToAnimationFinished(HideAnim, AnimationFinished);
    }
}

void UGameModeSelection::OnSinglePlayerClicked()
{
    PendingState = ETransitionState::Single;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"));
    }
}

void UGameModeSelection::OnMultiplayerClicked()
{
    PendingState = ETransitionState::Multiplayer;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else if (ServerBrowserWidgetClass)
    {
        RemoveFromParent();
        UServerBrowserWidget* Browser = CreateWidget<UServerBrowserWidget>(GetWorld(), ServerBrowserWidgetClass);
        Browser->AddToViewport();
    }
}

void UGameModeSelection::OnBackClicked()
{
    PendingState = ETransitionState::Back;
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else if (MainMenuClass)
    {
        RemoveFromParent();
        UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
        Menu->AddToViewport();
    }
}

void UGameModeSelection::OnHideAnimationFinished()
{
    switch (PendingState)
    {
    case ETransitionState::Single:
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"));
        break;
    case ETransitionState::Multiplayer:
        if (ServerBrowserWidgetClass)
        {
            UServerBrowserWidget* Browser = CreateWidget<UServerBrowserWidget>(GetWorld(), ServerBrowserWidgetClass);
            Browser->AddToViewport();
        }
        break;
    case ETransitionState::Back:
        if (MainMenuClass)
        {
            UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
            Menu->AddToViewport();
        }
        break;
    default:
        break;
    }
    PendingState = ETransitionState::None;
}

void UGameModeSelection::OnShownimationFinished()
{
	PlayAnimation(ShowAnim);
}
