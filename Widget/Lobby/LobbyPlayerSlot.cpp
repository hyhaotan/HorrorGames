#include "LobbyPlayerSlot.h"
#include "InviteFriendsWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"

void ULobbyPlayerSlot::NativeConstruct()
{
    Super::NativeConstruct();
    SlotIndex = -1;
    bIsHost = false;
    Ping = 0.0f;
    SetEmpty();
}

void ULobbyPlayerSlot::SetPlayerInfo(const FString& PlayerName, bool bInIsReady)
{
    CurrentPlayerName = PlayerName;
    bIsReady = bInIsReady;
    bIsOccupied = true;
    UpdateSlotContent();
}

void ULobbyPlayerSlot::SetEmpty()
{
    CurrentPlayerName = TEXT("");
    bIsReady = false;
    bIsOccupied = false;
    bIsHost = false;
    Ping = 0.0f;
    UpdateSlotContent();
}

void ULobbyPlayerSlot::SetIsHost(bool bInIsHost)
{
    bIsHost = bInIsHost;
    UpdateSlotContent();
}

void ULobbyPlayerSlot::SetPing(float InPing)
{
    Ping = InPing;
    UpdateSlotContent();
}

void ULobbyPlayerSlot::UpdateSlotContent()
{
    // Update player name
    if (PlayerNameText)
    {
        if (bIsOccupied)
        {
            FString DisplayName = CurrentPlayerName;
            if (bIsHost)
            {
                DisplayName += TEXT("(Host)");
            }
            PlayerNameText->SetText(FText::FromString(DisplayName));
            PlayerNameText->SetColorAndOpacity(bIsHost ? FLinearColor::Black : FLinearColor::White);
        }
        else
        {
            PlayerNameText->SetText(FText::FromString(TEXT("Empty Slot")));
            PlayerNameText->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
        }
    }

    // Update status
    if (StatusText)
    {
        if (bIsOccupied)
        {
            FString StatusString = bIsReady ? TEXT("Ready") : TEXT("Not Ready");
            StatusText->SetText(FText::FromString(StatusString));
            StatusText->SetColorAndOpacity(bIsReady ? FLinearColor::Green : FLinearColor::Red);
        }
        else
        {
            StatusText->SetText(FText::FromString(TEXT("Waiting...")));
            StatusText->SetColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
        }
    }

    // Update avatar/border
    if (SlotBorder)
    {
        FLinearColor BorderColor;
        if (bIsOccupied)
        {
            if (bIsHost)
                BorderColor = FLinearColor::Yellow;
            else if (bIsReady)
                BorderColor = FLinearColor::Green;
            else
                BorderColor = FLinearColor::Red;
        }
        else
        {
            BorderColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
        }
        SlotBorder->SetBrushColor(BorderColor);
    }

    // Update ping display
    if (PingText)
    {
        if (bIsOccupied && Ping > 0)
        {
            FString PingString = FString::Printf(TEXT("%.0fms"), Ping);
            PingText->SetText(FText::FromString(PingString));

            FLinearColor PingColor;
            if (Ping < 50) PingColor = FLinearColor::Green;
            else if (Ping < 100) PingColor = FLinearColor::Yellow;
            else PingColor = FLinearColor::Red;

            PingText->SetColorAndOpacity(PingColor);
            PingText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            PingText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Update invite widget
    if (InviteWidget)
    {
        InviteWidget->SetOccupied(bIsOccupied);
    }
}