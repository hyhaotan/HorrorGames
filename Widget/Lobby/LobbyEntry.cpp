#include "LobbyEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void ULobbyEntry::NativeConstruct()
{
    Super::NativeConstruct();

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &ULobbyEntry::OnJoinButtonClicked);
    }
}

void ULobbyEntry::SetLobbyData(const FString& InLobbyData)
{
    LobbyData = InLobbyData;

    if (LobbyNameText)
    {
        LobbyNameText->SetText(FText::FromString(LobbyData));
    }
}

void ULobbyEntry::OnJoinButtonClicked()
{
    OnLobbyJoinRequested.Broadcast(LobbyData);
}