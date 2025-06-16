#include "ServerBrowserWidget.h"
#include "SessionRowWidget.h"
#include "HorrorGame/Widget/Menu/GameModeSelection.h"
#include "HorrorGame/Widget/SessionBrowser/CreateSessionWidget.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemUtils.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"   // Đảm bảo include cho EditableTextBox
#include "Kismet/GameplayStatics.h"

// Gọi khi widget được add vào viewport
void UServerBrowserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RefreshButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnRefreshClicked);
    CreateLobbyButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnCreateLobbyClicked);
    SearchButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::FindSessions);
	BackButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::OnBackWidget);

    // Khởi delegate cho Find / Join / Create
    OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UServerBrowserWidget::OnFindSessionsComplete);
    OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UServerBrowserWidget::OnJoinSessionComplete);
    OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UServerBrowserWidget::OnCreateSessionComplete);

    // Tự động gọi FindSessions khi widget tạo xong
    FindSessions();
}

IOnlineSessionPtr UServerBrowserWidget::GetSessionInterface() const
{
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("Steam")))
    {
        return OSS->GetSessionInterface();
    }
    return nullptr;
}

void UServerBrowserWidget::OnRefreshClicked()
{
    // Nếu có EditBox, clear nội dung
    if (FindSessionTextBox)
    {
        FindSessionTextBox->SetText(FText::GetEmpty());
    }

    // Thực sự gọi FindSessions. Ở lần này, vì FindSessionTextBox rỗng, FindSessions() sẽ không set filter
    FindSessions();
}

void UServerBrowserWidget::OnBackWidget()
{
    this->RemoveFromParent();

    if (GameModeSelectionClass)
    {
        UGameModeSelection* GameModeWidget = CreateWidget<UGameModeSelection>(GetWorld(), GameModeSelectionClass);
        if (GameModeWidget)
        {
            GameModeWidget->ShowAnimGameMode();
            GameModeWidget->AddToViewport();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] MainMenuWidgetClass chưa được gán!"));
	}
}

void UServerBrowserWidget::FindSessions()
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] SessionInterface không hợp lệ"));
        return;
    }

    // Nếu đang có search cũ, unregister callback trước
    if (OnFindSessionsCompleteDelegateHandle.IsValid())
    {
        Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = false;                   // false nếu xài Steam
    SessionSearch->MaxSearchResults = 100;                 // Giới hạn số kết quả
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    // === BẮT ĐẦU PHẦN MỚI: LẤY TỪ KHÓA TỪ FindSessionTextBox ===
    if (FindSessionTextBox)
    {
        FString SearchKey = FindSessionTextBox->GetText().ToString().TrimStartAndEnd();
        if (!SearchKey.IsEmpty())
        {
            // Filter theo RoomNameKey
            SessionSearch->QuerySettings.Set(
                UGameModeSelection::RoomNameKey,
                SearchKey,
                EOnlineComparisonOp::Equals
            );

            UE_LOG(
                LogTemp,
                Log,
                TEXT("[ServerBrowser] Tìm session với từ khóa RoomName = %s"),
                *SearchKey
            );
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("[ServerBrowser] Từ khóa tìm kiếm trống, hiển thị tất cả session"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] FindSessionTextBox không hợp lệ (nullptr)"));
    }
    // === KẾT THÚC PHẦN MỚI ===

    OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
    Sessions->FindSessions(0, SessionSearch.ToSharedRef());

    UE_LOG(LogTemp, Log, TEXT("[ServerBrowser] Bắt đầu tìm session..."));
}

void UServerBrowserWidget::OnFindSessionsComplete(bool bWasSuccessful)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid())
        return;

    // Unbind delegate
    Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

    // Nếu thất bại hoặc không có kết quả
    if (!bWasSuccessful || !SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] FindSessions thất bại hoặc không có kết quả"));
        return;
    }

    // Clear danh sách cũ trong ScrollBox
    if (SessionListScrollBox)
    {
        SessionListScrollBox->ClearChildren();
    }

    const TArray<FOnlineSessionSearchResult>& SearchResults = SessionSearch->SearchResults;
    for (int32 i = 0; i < SearchResults.Num(); ++i)
    {
        const FOnlineSessionSearchResult& Result = SearchResults[i];

        // Tạo 1 SessionRowWidget cho mỗi kết quả
        if (SessionRowWidgetClass)
        {
            USessionRowWidget* Row = CreateWidget<USessionRowWidget>(GetWorld(), SessionRowWidgetClass);
            if (Row)
            {
                Row->SetupRow(Result, i);
                // Bind delegate khi row báo index
                Row->OnJoinSessionClicked.AddUniqueDynamic(this, &UServerBrowserWidget::JoinSessionByIndex);

                SessionListScrollBox->AddChild(Row);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[ServerBrowser] Tìm được %d session"), SearchResults.Num());
}

void UServerBrowserWidget::JoinSessionByIndex(int32 Index)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid() || !SessionSearch.IsValid())
        return;

    if (!SessionSearch->SearchResults.IsValidIndex(Index))
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] Không thể join session index %d (không hợp lệ)"), Index);
        return;
    }

    // Xóa callback join cũ (nếu có)
    Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

    // Bind delegate join
    OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

    // Gọi JoinSession
    Sessions->JoinSession(0, UGameModeSelection::SessionName, SessionSearch->SearchResults[Index]);
    UE_LOG(LogTemp, Log, TEXT("[ServerBrowser] Đang join session index %d"), Index);
}

void UServerBrowserWidget::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSessionPtr Sessions = GetSessionInterface();
    if (!Sessions.IsValid())
        return;

    Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Error, TEXT("[ServerBrowser] JoinSession thất bại: %d"), (int32)Result);
        return;
    }

    // Lấy connect string (ví dụ "127.0.0.1:7777")
    FString ConnectInfo;
    if (Sessions->GetResolvedConnectString(InSessionName, ConnectInfo))
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->ClientTravel(ConnectInfo, TRAVEL_Absolute);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[ServerBrowser] Không lấy được ConnectString để travel"));
    }
}

void UServerBrowserWidget::OnCreateLobbyClicked()
{
    // Hiện CreateSessionWidget
    if (CreateSessionWidgetClass)
    {
        UCreateSessionWidget* CreateWdg = CreateWidget<UCreateSessionWidget>(GetWorld(), CreateSessionWidgetClass);
        if (CreateWdg)
        {
            CreateWdg->OnCreateSessionRequest.AddDynamic(this, &UServerBrowserWidget::OnCreateSessionRequest);
            CreateWdg->AddToViewport(100);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] CreateSessionWidgetClass chưa được gán!"));
    }
}

void UServerBrowserWidget::OnCreateSessionRequest(const FSessionSettingsData& SessionData)
{
    // Khi user bấm Create trong CreateSessionWidget, đã khởi tạo session mới.
    // Trước hết, remove CreateSessionWidget khỏi viewport
    if (UUserWidget* ParentWidget = Cast<UUserWidget>(GetParent()))
    {
        // Tuy nhiên vì CreateSessionWidget được add trực tiếp, dùng cách này để xóa hết
    }
    // Tạo session mới (host) với thông tin SessionData
    CreateNewSession(SessionData);
}

void UServerBrowserWidget::CreateNewSession(const FSessionSettingsData& SessionData)
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("Steam"));
    if (!OSS)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] Không tìm thấy OnlineSubsystem Steam"));
        return;
    }

    IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
    if (!Sessions.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerBrowser] SessionInterface không hợp lệ"));
        return;
    }

    // Bind callback
    Sessions->OnCreateSessionCompleteDelegates.AddUObject(this, &UServerBrowserWidget::OnCreateSessionComplete);

    // Cấu hình session settings
    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = SessionData.MaxPlayers;
    Settings.bIsLANMatch = false;
    Settings.bUsesPresence = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bShouldAdvertise = true;
    Settings.bAllowJoinViaPresence = true;

    Settings.Set(UGameModeSelection::RoomNameKey, SessionData.RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
    Settings.Set(UGameModeSelection::PasswordKey, SessionData.Password, EOnlineDataAdvertisementType::ViaOnlineService);
    Settings.Set(UGameModeSelection::MapNameKey, SessionData.MapName, EOnlineDataAdvertisementType::ViaOnlineService);

    UE_LOG(LogTemp, Log, TEXT("[ServerBrowser] Create session room=%s, pw=%s, max=%d, map=%s"),
        *SessionData.RoomName,
        *SessionData.Password,
        SessionData.MaxPlayers,
        *SessionData.MapName);

    Sessions->CreateSession(0, UGameModeSelection::SessionName, Settings);
}

void UServerBrowserWidget::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("[ServerBrowser] Tạo session thất bại"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[ServerBrowser] CreateSession thành công, chuyển sang LobbyMap"));
    UGameplayStatics::OpenLevel(this, TEXT("LobbyMap"), true, TEXT("?listen"));
}
