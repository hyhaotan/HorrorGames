#include "SessionRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "HorrorGame/Widget/Menu/GameModeSelection.h"

void USessionRowWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &USessionRowWidget::OnJoinButtonClicked);
    }
}

void USessionRowWidget::SetupRow(const FOnlineSessionSearchResult& InSearchResult, int32 InIndex)
{
    RowIndex = InIndex;

    // Lấy RoomName
    FString RoomName;
    InSearchResult.Session.SessionSettings.Get(UGameModeSelection::RoomNameKey, RoomName);
    if (RoomNameText)
    {
        RoomNameText->SetText(FText::FromString(RoomName));
    }

    // Lấy MapName
    FString MapName;
    InSearchResult.Session.SessionSettings.Get(UGameModeSelection::MapNameKey, MapName);
    if (MapNameText)
    {
        MapNameText->SetText(FText::FromString(MapName));
    }

    // Lấy số người hiện có / tối đa
    int32 MaxPlayers = InSearchResult.Session.SessionSettings.NumPublicConnections;
    int32 OpenSeats = InSearchResult.Session.NumOpenPublicConnections;
    int32 CurrentPlayers = MaxPlayers - OpenSeats;

    if (PlayerCountText)
    {
        FString CountStr = FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers);
        PlayerCountText->SetText(FText::FromString(CountStr));
    }
}

void USessionRowWidget::OnJoinButtonClicked()
{
    // Bắn event kèm index của session để ServerBrowserWidget xử lý Join
    OnJoinSessionClicked.Broadcast(RowIndex);
}
