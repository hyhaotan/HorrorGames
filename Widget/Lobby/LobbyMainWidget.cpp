#include "LobbyMainWidget.h"
#include "LobbyPlayerSlot.h"
#include "InviteFriendsWidget.h"
#include "FriendInviteWidget.h"
#include "LobbyPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void ULobbyMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    bIsReady = false;
    bIsHost = false;
    CurrentState = ELobbyState::Waiting;

    // Bind button events
    if (ReadyButton)
    {
        ReadyButton->OnClicked.AddDynamic(this, &ULobbyMainWidget::OnReadyButtonClicked);
    }

    if (StartGameButton)
    {
        StartGameButton->OnClicked.AddDynamic(this, &ULobbyMainWidget::OnStartGameButtonClicked);
    }

    if (LeaveLobbyButton)
    {
        LeaveLobbyButton->OnClicked.AddDynamic(this, &ULobbyMainWidget::OnLeaveLobbyButtonClicked);
    }

    if (SaveSettingsButton)
    {
        SaveSettingsButton->OnClicked.AddDynamic(this, &ULobbyMainWidget::OnSaveSettingsButtonClicked);
    }

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;

        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(this->TakeWidget()); 
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }

    CreatePlayerSlots();
    InitializeSettingsPanel();
    UpdateReadyButtonText();
    UpdateStartGameButtonState();

    SetIsFocusable(true);
}

void ULobbyMainWidget::NativeDestruct()
{
    Super::NativeDestruct();

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        PC->bShowMouseCursor = false;
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false;

        FInputModeGameOnly GameInput;
        PC->SetInputMode(GameInput);

        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
    }
}

void ULobbyMainWidget::CreatePlayerSlots()
{
    if (!PlayerSlotsContainer || !PlayerSlotWidgetClass) return;

    PlayerSlotsContainer->ClearChildren();
    PlayerSlots.Empty();

    // Create 4 slots (or max players from settings)
    for (int32 i = 0; i < 4; i++)
    {
        ULobbyPlayerSlot* PlayerSlot = CreateWidget<ULobbyPlayerSlot>(this, PlayerSlotWidgetClass);
        if (IsValid(PlayerSlot))
        {
            PlayerSlot->SlotIndex = i;
            PlayerSlot->SetEmpty();
            PlayerSlotsContainer->AddChildToVerticalBox(PlayerSlot);
            PlayerSlots.Add(PlayerSlot);

            // Create invite widget for each slot
            if (InviteFriendWidgetClass)
            {
                UInviteFriendsWidget* InviteWidget = CreateWidget<UInviteFriendsWidget>(this, InviteFriendWidgetClass);
                if (IsValid(InviteWidget))
                {
                    InviteWidget->SlotIndex = i;
                    InviteWidget->OnInviteClicked.AddDynamic(this, &ULobbyMainWidget::OnInviteSlotClicked);
                    InviteWidget->OnLeaveClicked.AddDynamic(this, &ULobbyMainWidget::OnLeaveSlotClicked);
                    PlayerSlot->InviteWidget = InviteWidget;
                }
            }
        }
    }
}

void ULobbyMainWidget::UpdateLobbyInfo(const FLobbySettings& Settings)
{
    if (LobbyNameText)
    {
        LobbyNameText->SetText(FText::FromString(Settings.LobbyName));
    }

    // Update settings panel
    if (LobbyNameInput)
    {
        LobbyNameInput->SetText(FText::FromString(Settings.LobbyName));
    }

    if (PrivateLobbyCheckbox)
    {
        PrivateLobbyCheckbox->SetIsChecked(Settings.bIsPrivate);
    }

    if (GameModeCombo)
    {
        GameModeCombo->SetSelectedOption(Settings.GameMode);
    }

    if (MapCombo)
    {
        MapCombo->SetSelectedOption(Settings.MapName);
    }
}

void ULobbyMainWidget::UpdatePlayerSlots(const TArray<FLobbyPlayerData>& Players)
{
    // Reset all slots
    for (ULobbyPlayerSlot* Slots : PlayerSlots)
    {
        if (IsValid(Slots))
        {
            Slots->SetEmpty();
            if (Slots->InviteWidget)
            {
                Slots->InviteWidget->SetOccupied(false);
            }
        }
    }

    // Fill slots with players
    for (const FLobbyPlayerData& Player : Players)
    {
        if (Player.SlotIndex >= 0 && Player.SlotIndex < PlayerSlots.Num())
        {
            ULobbyPlayerSlot* Slotss = PlayerSlots[Player.SlotIndex];
            if (IsValid(Slotss))
            {
                FString DisplayName = FString::Printf(TEXT("%s (%s)"), *Player.DisplayName, *Player.PlayerID);
                bool bReady = (Player.ReadyState == EPlayerReadyState::Ready);

                Slotss->SetPlayerInfo(DisplayName, bReady);
                Slotss->SetIsHost(Player.bIsHost);
                Slotss->SetPing(Player.Ping);

                if (Slotss->InviteWidget)
                {
                    Slotss->InviteWidget->SetOccupied(true);
                }
            }
        }
    }

    // Update player count
    if (PlayerCountText)
    {
        FString CountText = FString::Printf(TEXT("%d / %d Players"), Players.Num(), 4);
        PlayerCountText->SetText(FText::FromString(CountText));
    }

    UpdateStartGameButtonState();
}

void ULobbyMainWidget::UpdateLobbyState(ELobbyState NewState)
{
    CurrentState = NewState;

    FString StateText;
    switch (NewState)
    {
    case ELobbyState::Waiting:
        StateText = TEXT("Waiting for players...");
        break;
    case ELobbyState::CountingDown:
        StateText = TEXT("Starting soon...");
        break;
    case ELobbyState::Starting:
        StateText = TEXT("Game Starting!");
        break;
    case ELobbyState::InGame:
        StateText = TEXT("In Game");
        break;
    }

    if (LobbyStateText)
    {
        LobbyStateText->SetText(FText::FromString(StateText));
    }

    UpdateStartGameButtonState();
}

void ULobbyMainWidget::UpdateCountdown(int32 SecondsLeft)
{
    if (CountdownText)
    {
        if (SecondsLeft > 0)
        {
            FString CountdownString = FString::Printf(TEXT("Starting in: %d"), SecondsLeft);
            CountdownText->SetText(FText::FromString(CountdownString));
            CountdownText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            CountdownText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void ULobbyMainWidget::SetHostControls(bool bInIsHost)
{
    bIsHost = bInIsHost;

    // Show/hide host-only controls
    if (StartGameButton)
    {
        StartGameButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    // Settings panel visibility
    if (SaveSettingsButton)
    {
        SaveSettingsButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (LobbyNameInput)
    {
        LobbyNameInput->SetIsReadOnly(!bIsHost);
    }

    if (PrivateLobbyCheckbox)
    {
        PrivateLobbyCheckbox->SetIsEnabled(bIsHost);
    }

    if (GameModeCombo)
    {
        GameModeCombo->SetIsEnabled(bIsHost);
    }

    if (MapCombo)
    {
        MapCombo->SetIsEnabled(bIsHost);
    }

    UpdateStartGameButtonState();
}

void ULobbyMainWidget::OnReadyButtonClicked()
{
    bIsReady = !bIsReady;

    ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer());
    if (IsValid(PC))
    {
        PC->SetPlayerReady(bIsReady);
    }

    UpdateReadyButtonText();
}

void ULobbyMainWidget::OnStartGameButtonClicked()
{
    if (!bIsHost) return;

    ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer());
    if (IsValid(PC))
    {
        PC->StartGame();
    }
}

void ULobbyMainWidget::OnLeaveLobbyButtonClicked()
{
    ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer());
    if (IsValid(PC))
    {
        PC->LeaveLobby();
    }
}

void ULobbyMainWidget::OnSaveSettingsButtonClicked()
{
    if (!bIsHost) return;

    FLobbySettings NewSettings;

    if (LobbyNameInput)
    {
        NewSettings.LobbyName = LobbyNameInput->GetText().ToString();
    }

    if (PrivateLobbyCheckbox)
    {
        NewSettings.bIsPrivate = PrivateLobbyCheckbox->IsChecked();
    }

    if (PasswordInput)
    {
        NewSettings.Password = PasswordInput->GetText().ToString();
    }

    if (GameModeCombo)
    {
        NewSettings.GameMode = GameModeCombo->GetSelectedOption();
    }

    if (MapCombo)
    {
        NewSettings.MapName = MapCombo->GetSelectedOption();
    }

    NewSettings.MaxPlayers = 4; // Fixed for this example

    ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer());
    if (IsValid(PC))
    {
        PC->UpdateLobbySettings(NewSettings);
    }
}

void ULobbyMainWidget::OnInviteSlotClicked(int32 SlotIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Opening friend invite for slot %d"), SlotIndex);

    // Create and show friend invite widget
    UFriendInviteWidget* FriendWidget = CreateWidget<UFriendInviteWidget>(this, UFriendInviteWidget::StaticClass());
    if (IsValid(FriendWidget))
    {
        FriendWidget->AddToViewport();
        FriendWidget->RefreshFriendsList();
    }
}

void ULobbyMainWidget::OnLeaveSlotClicked(int32 SlotIndex)
{
    // This would kick a player if host, or leave if it's the current player
    UE_LOG(LogTemp, Log, TEXT("Leave/Kick action for slot %d"), SlotIndex);
}

void ULobbyMainWidget::UpdateReadyButtonText()
{
    if (ReadyButton && ReadyButton->GetContent())
    {
        UTextBlock* TB = Cast<UTextBlock>(ReadyButton->GetContent());
        if (TB)
        {
            FText ButtonText = bIsReady ? FText::FromString(TEXT("Not Ready")) : FText::FromString(TEXT("Ready"));
            TB->SetText(ButtonText);
        }
    }
}

void ULobbyMainWidget::UpdateStartGameButtonState()
{
    if (StartGameButton)
    {
        bool bCanStart = bIsHost && (CurrentState == ELobbyState::Waiting || CurrentState == ELobbyState::CountingDown);
        StartGameButton->SetIsEnabled(bCanStart);
    }
}

void ULobbyMainWidget::InitializeSettingsPanel()
{
    // Initialize combo boxes
    if (GameModeCombo)
    {
        GameModeCombo->AddOption(TEXT("Classic"));
        GameModeCombo->AddOption(TEXT("Hardcore"));
        GameModeCombo->AddOption(TEXT("Quick Match"));
        GameModeCombo->SetSelectedOption(TEXT("Classic"));
    }

    if (MapCombo)
    {
        MapCombo->AddOption(TEXT("Abandoned Hospital"));
        MapCombo->AddOption(TEXT("Dark Forest"));
        MapCombo->AddOption(TEXT("Haunted Mansion"));
        MapCombo->AddOption(TEXT("Underground Tunnels"));
        MapCombo->SetSelectedOption(TEXT("Abandoned Hospital"));
    }
}