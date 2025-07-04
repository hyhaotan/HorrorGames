#include "ServerBrowserWidget.h"
#include "SessionRowWidget.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "HorrorGame/Widget/Menu/GameModeSelection.h"
#include "Online/OnlineSessionNames.h"
#include "CreateSessionWidget.h"

// Define static constants
const FName UServerBrowserWidget::Name_GameSession = TEXT("GameSession");
const FName UServerBrowserWidget::RoomKey = TEXT("ROOM_NAME");
const FName UServerBrowserWidget::PasswordKey = TEXT("PASSWORD");
const FName UServerBrowserWidget::MapKey = TEXT("MAP_NAME");

void UServerBrowserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RefreshButton)
        RefreshButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnRefreshClicked);
    if (BackButton)
        BackButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnBackClicked);
    if (CreateLobbyButton)
        CreateLobbyButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnCreateLobbyClicked);

    FindSessions();
}

IOnlineSessionPtr UServerBrowserWidget::GetSessionInterface() const
{
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(FName("Steam")))
        return OSS->GetSessionInterface();
    return nullptr;
}

void UServerBrowserWidget::OnRefreshClicked()
{
    FindSessions(SearchBox ? SearchBox->GetText().ToString().TrimStartAndEnd() : TEXT(""));
}

void UServerBrowserWidget::OnBackClicked()
{
    this->RemoveFromParent();
    if (GameModeSelectionClass)
    {
        UGameModeSelection* GameModeWidget = CreateWidget<UGameModeSelection>(GetWorld(), GameModeSelectionClass);
        if (GameModeWidget)
        {
            GameModeWidget->OnShownimationFinished();
            GameModeWidget->AddToViewport();
        }
    }
}

void UServerBrowserWidget::FindSessions(const FString& Filter)
{
    if (IOnlineSessionPtr Sessions = GetSessionInterface())
    {
        Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);

        SearchSettings = MakeShared<FOnlineSessionSearch>();
        SearchSettings->bIsLanQuery = false;
        SearchSettings->MaxSearchResults = 100;
        SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
        if (!Filter.IsEmpty())
            SearchSettings->QuerySettings.Set(RoomKey, Filter, EOnlineComparisonOp::Equals);

        FindHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
            FOnFindSessionsCompleteDelegate::CreateUObject(this, &UServerBrowserWidget::OnFindSessionsComplete)
        );
        Sessions->FindSessions(0, SearchSettings.ToSharedRef());
    }
}

void UServerBrowserWidget::OnFindSessionsComplete(bool bSuccess)
{
    if (IOnlineSessionPtr Sessions = GetSessionInterface())
        Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
    if (!bSuccess || !SearchSettings.IsValid()) return;

    if (SessionListBox)
        SessionListBox->ClearChildren();

    for (int32 i = 0; i < SearchSettings->SearchResults.Num(); ++i)
    {
        const auto& Result = SearchSettings->SearchResults[i];
        FSessionSettingsData Data;
        Result.Session.SessionSettings.Get(RoomKey, Data.RoomName);
        Result.Session.SessionSettings.Get(PasswordKey, Data.Password);
        Data.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
        Result.Session.SessionSettings.Get(MapKey, Data.MapName);

        if (!SessionRowClass) continue;
        USessionRowWidget* Row = CreateWidget<USessionRowWidget>(GetWorld(), SessionRowClass);
        Row->Setup(Data, i);
        Row->OnJoinSessionClicked.AddDynamic(this, &UServerBrowserWidget::JoinSession);
        SessionListBox->AddChild(Row);
    }
}

void UServerBrowserWidget::JoinSession(int32 Index)
{
    if (IOnlineSessionPtr Sessions = GetSessionInterface())
    {
        Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
        JoinHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &UServerBrowserWidget::OnJoinSessionComplete)
        );
        Sessions->JoinSession(0, Name_GameSession, SearchSettings->SearchResults[Index]);
    }
}

void UServerBrowserWidget::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (Sessions.IsValid())
    {
        Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
    }

    if (Result != EOnJoinSessionCompleteResult::Success) return;

    FString Url;
    if (Sessions.IsValid() && Sessions->GetResolvedConnectString(InSessionName, Url))
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PC->ClientTravel(Url, TRAVEL_Absolute);
        }
    }
}

void UServerBrowserWidget::OnCreateLobbyClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("OnCreateLobbyClicked fired!"));
    UCreateSessionWidget* CreateWidgetInstance = CreateWidget<UCreateSessionWidget>(GetWorld(), CreateSessionWidgetClass);
    if (CreateWidgetInstance)
    {
        CreateWidgetInstance->AddToViewport();
    }
}