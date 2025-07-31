#include "LobbyPlayerSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSessionSettings.h"

void ULobbyPlayerSlot::SetPlayerInfo(const FString& PlayerName, bool bInIsHost)
{
    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(PlayerName));
    }

    if (StatusText)
    {
        StatusText->SetText(FText::FromString(bInIsHost ? TEXT("Host") : TEXT("Player")));
    }

    bIsOccupied = true;
    bIsHost = bInIsHost;
    
    HandleInviteButtonVisibility();
}

void ULobbyPlayerSlot::SetEmpty()
{
    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(TEXT("Empty Slot")));
    }

    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("Waiting...")));
    }

    bIsOccupied = false;
    bIsHost = false;

    HandleInviteButtonVisibility();
}

void ULobbyPlayerSlot::HandleInviteButtonVisibility()
{
    if (InviteButton)
    {
        InviteButton->SetVisibility(bIsOccupied ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
}

void ULobbyPlayerSlot::OnInviteButtonClicked()
{
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