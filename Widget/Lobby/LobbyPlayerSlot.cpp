#include "LobbyPlayerSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"

void ULobbyPlayerSlot::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (InviteButton)
    {
        InviteButton->OnClicked.AddDynamic(this, &ULobbyPlayerSlot::OnInviteButtonClicked);
    }

    if (KickButton)
    {
        KickButton->OnClicked.AddDynamic(this, &ULobbyPlayerSlot::OnKickButtonClicked);
    }

    // Initialize as empty
    SetEmpty();
}

void ULobbyPlayerSlot::SetPlayerInfo(const FString& PlayerName, bool bInIsHost)
{
    CurrentPlayerName = PlayerName;
    bIsOccupied = true;
    bIsHost = bInIsHost;

    // Update UI elements
    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(PlayerName));
    }

    if (StatusText)
    {
        FString StatusString = bIsHost ? TEXT("Host") : TEXT("Player");
        StatusText->SetText(FText::FromString(StatusString));
    }

    // Show/hide host icon
    if (HostIcon)
    {
        HostIcon->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }

    UpdateButtonVisibility();
}

void ULobbyPlayerSlot::SetEmpty()
{
    CurrentPlayerName = TEXT("");
    bIsOccupied = false;
    bIsHost = false;

    // Update UI elements
    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(TEXT("Empty Slot")));
    }

    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("Waiting for player...")));
    }

    // Hide host icon
    if (HostIcon)
    {
        HostIcon->SetVisibility(ESlateVisibility::Hidden);
    }

    // Clear avatar (could set to default empty avatar)
    if (PlayerAvatar)
    {
        // PlayerAvatar->SetBrushFromTexture(DefaultEmptyAvatar);
    }

    UpdateButtonVisibility();
}

void ULobbyPlayerSlot::UpdateButtonVisibility()
{
    // Show invite button only on empty slots
    if (InviteButton)
    {
        InviteButton->SetVisibility(bIsOccupied ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }

    // Show kick button only for non-host players (and only if local player is host)
    if (KickButton)
    {
        bool bShowKick = bIsOccupied && !bIsHost; // && IsLocalPlayerHost();
        KickButton->SetVisibility(bShowKick ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void ULobbyPlayerSlot::OnInviteButtonClicked()
{
    // Open Steam friends UI for inviting
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(TEXT("Steam"));
    if (OnlineSubsystem)
    {
        IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
        if (ExternalUI.IsValid())
        {
            ExternalUI->ShowFriendsUI(0);
        }
    }
}

void ULobbyPlayerSlot::OnKickButtonClicked()
{
    // TODO: Implement kick functionality
    // This would require server authority to kick players
    UE_LOG(LogTemp, Log, TEXT("Kick button clicked for player: %s"), *CurrentPlayerName);
}