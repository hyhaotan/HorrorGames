#include "GameModeSelection.h"
#include "LobbyWidget.h"
#include "MainMenu.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionClient.h"
#include "Components/Button.h"

void UGameModeSelection::NativeConstruct()
{
    Super::NativeConstruct();

    if (ShowAnim) PlayAnimation(ShowAnim);
    if (SinglePlayerButton) SinglePlayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnSinglePlayerButtonClicked);
    if (MultiplayerButton) MultiplayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnMultiplayerButtonClicked);
    if (BackButton) BackButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnBackButtonClicked);
    if (HideAnim)
    {
        FWidgetAnimationDynamicEvent AnimFinishedDelegate;
        AnimFinishedDelegate.BindDynamic(this, &UGameModeSelection::OnHideAnimationFinished);
        BindToAnimationFinished(HideAnim, AnimFinishedDelegate);
    }
    CurrentState = EGameModeState::None;
}

void UGameModeSelection::OnSinglePlayerButtonClicked()
{
    // Đánh dấu trạng thái SinglePlayer
    CurrentState = EGameModeState::Single;

    // Nếu có hide animation thì play, nếu không thì load ngay
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        StartSinglePlayer();
    }
}

void UGameModeSelection::OnMultiplayerButtonClicked()
{
    // Đánh dấu trạng thái Multiplayer
    CurrentState = EGameModeState::Multiplayer;

    // Nếu có hide animation thì play, nếu không thì start tạo session ngay
    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        StartMultiplayer();
    }
}

void UGameModeSelection::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("[GameModeSelection] Tạo session thất bại"));
        // Có thể show một thông báo popup cho người chơi
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[GameModeSelection] CreateSession thành công, chuyển sang LobbyMap"));

    // Mở LobbyMap ở chế độ listen (server)
    UGameplayStatics::OpenLevel(this, TEXT("LobbyMap"), true, TEXT("?listen"));
}

void UGameModeSelection::OnBackButtonClicked()
{
    // Đánh dấu trạng thái Back
    CurrentState = EGameModeState::Back;

    if (HideAnim)
    {
        PlayAnimation(HideAnim);
    }
    else
    {
        // Nếu không có animation, remove widget ngay và show MainMenu
        RemoveFromParent();
        if (MainMenuClass)
        {
            UMainMenu* Menu = CreateWidget<UMainMenu>(this, MainMenuClass);
            if (Menu)
            {
                Menu->AddToViewport();
            }
        }
    }
}

void UGameModeSelection::OnHideAnimationFinished()
{
    switch (CurrentState)
    {
        case EGameModeState::Single:
        {
            // Remove widget này
            RemoveFromParent();
            // Bắt đầu SinglePlayer
            StartSinglePlayer();
            break;
        }
        case EGameModeState::Multiplayer:
        {
            RemoveFromParent();
            // Bắt đầu Multiplayer (tạo session hoặc open lobby)
            StartMultiplayer();
            break;
        }
        case EGameModeState::Back:
        {
            RemoveFromParent();
            // Chuyển lại MainMenu
            if (MainMenuClass)
            {
                UMainMenu* Menu = CreateWidget<UMainMenu>(this, MainMenuClass);
                if (Menu)
                {
                    Menu->AddToViewport();
                }
            }
            break;
        }
        default:
        {
            // Nếu vì lý do nào đó trạng thái rỗng, mặc định remove
            RemoveFromParent();
            break;
        }
    }

    // Sau khi đã xử lý xong, reset trạng thái
    CurrentState = EGameModeState::None;
}

void UGameModeSelection::StartSinglePlayer()
{
    UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"),true);
}

void UGameModeSelection::StartMultiplayer()
{
    // 1) Lấy Steam subsystem
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("Steam"));
    if (!OSS)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GameModeSelection] Không tìm thấy OnlineSubsystem Steam"));
        return;
    }

    // 2) Lấy Session interface
    IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
    if (!Sessions.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[GameModeSelection] SessionInterface không hợp lệ"));
        return;
    }

    // 3) Bind delegate để đợi callback khi CreateSession xong
    Sessions->OnCreateSessionCompleteDelegates.AddUObject(this, &UGameModeSelection::OnCreateSessionComplete);

    // 4) Cấu hình Settings cho session: số người, LAN hay không, presence, quảng bá...
    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = 4;
    Settings.bIsLANMatch = false;
    Settings.bUsesPresence = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bShouldAdvertise = true;
    Settings.bAllowJoinViaPresence = true;
    Settings.Set(ULobbyWidget::MapNameSettingKey, FString("LobbyMap"), EOnlineDataAdvertisementType::ViaOnlineService);

    // 5) Gửi CreateSession request lên Steam
    UE_LOG(LogTemp, Log, TEXT("[GameModeSelection] Gọi CreateSession lên Steam với tên session: %s"), *SessionName.ToString());
    Sessions->CreateSession(0, SessionName, Settings);
}
