#include "GameModeSelection.h"
#include "HorrorGame/Widget/SessionBrowser/ServerBrowserWidget.h"
#include "HorrorGame/Widget/SessionBrowser/CreateSessionWidget.h"
#include "HorrorGame/Widget/Menu/MainMenu.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionClient.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/Button.h"

// Định nghĩa hằng số static
const FName UGameModeSelection::SessionName = FName(TEXT("GameSession"));
const FName UGameModeSelection::RoomNameKey = FName(TEXT("ROOM_NAME"));
const FName UGameModeSelection::PasswordKey = FName(TEXT("PASSWORD"));
const FName UGameModeSelection::MapNameKey = FName(TEXT("MAP_NAME"));

void UGameModeSelection::NativeConstruct()
{
	Super::NativeConstruct();

	// Play animation khi load lần đầu (nếu có)
	if (ShowAnim)
	{
		PlayAnimation(ShowAnim);
	}

	// Bind menu buttons
	if (SinglePlayerButton)
	{
		SinglePlayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnSinglePlayerButtonClicked);
	}
	if (MultiplayerButton)
	{
		MultiplayerButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnMultiplayerButtonClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UGameModeSelection::OnBackButtonClicked);
	}

	// Bind Hide animation xong
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
	CurrentState = EGameModeState::Single;
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
	CurrentState = EGameModeState::Multiplayer;
	if (HideAnim)
	{
		PlayAnimation(HideAnim);
	}
	else
	{
		// Hiển ServerBrowserWidget thay vì CreateSessionWidget trực tiếp
		if (ServerBrowserWidgetClass)
		{
			UServerBrowserWidget* BrowserWidget = CreateWidget<UServerBrowserWidget>(GetWorld(), ServerBrowserWidgetClass);
			if (BrowserWidget)
			{
				// Truyền reference CreateSessionWidgetClass xuống cho ServerBrowserWidget
				BrowserWidget->CreateSessionWidgetClass = CreateSessionWidgetClass;
				BrowserWidget->AddToViewport();

				// Ẩn menu chính
				SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GameModeSelection] ServerBrowserWidgetClass chưa được gán!"));
		}
	}
}

void UGameModeSelection::OnBackButtonClicked()
{
	CurrentState = EGameModeState::Back;
	if (HideAnim)
	{
		PlayAnimation(HideAnim);
	}
	else
	{
		RemoveFromParent();
		if (MainMenuClass)
		{
			UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
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
		RemoveFromParent();
		StartSinglePlayer();
		break;
	}
	case EGameModeState::Multiplayer:
	{
		RemoveFromParent();
		if (ServerBrowserWidgetClass)
		{
			UServerBrowserWidget* BrowserWidget = CreateWidget<UServerBrowserWidget>(GetWorld(), ServerBrowserWidgetClass);
			if (BrowserWidget)
			{
				BrowserWidget->CreateSessionWidgetClass = CreateSessionWidgetClass;
				BrowserWidget->AddToViewport();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GameModeSelection] ServerBrowserWidgetClass chưa được gán!"));
		}
		break;
	}
	case EGameModeState::Back:
	{
		RemoveFromParent();
		if (MainMenuClass)
		{
			UMainMenu* Menu = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
			if (Menu)
			{
				Menu->AddToViewport();
			}
		}
		break;
	}
	default:
	{
		RemoveFromParent();
		break;
	}
	}

	CurrentState = EGameModeState::None;
}

void UGameModeSelection::OnCreateSessionRequest(const FSessionSettingsData& SessionData)
{
	// Nếu đang có CreateSessionWidget hiện lên, xóa nó
	if (ActiveCreateSessionWidget)
	{
		ActiveCreateSessionWidget->RemoveFromParent();
		ActiveCreateSessionWidget = nullptr;
	}

	// Tạo session với dữ liệu do user nhập
	StartMultiplayer(SessionData);
}

void UGameModeSelection::StartSinglePlayer()
{
	UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"), true);
}

void UGameModeSelection::StartMultiplayer(const FSessionSettingsData& SessionData)
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("Steam"));
	if (!OSS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameModeSelection] Không tìm thấy OnlineSubsystem Steam"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[GameModeSelection] Đã tìm thấy OnlineSubsystem Steam"));

	IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameModeSelection] SessionInterface không hợp lệ"));
		return;
	}

	// Bind callback
	Sessions->OnCreateSessionCompleteDelegates.AddUObject(this, &UGameModeSelection::OnCreateSessionComplete);

	// Cấu hình session settings
	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = SessionData.MaxPlayers;
	Settings.bIsLANMatch = false;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinViaPresence = true;

	Settings.Set(RoomNameKey, SessionData.RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
	Settings.Set(PasswordKey, SessionData.Password, EOnlineDataAdvertisementType::ViaOnlineService);
	Settings.Set(MapNameKey, SessionData.MapName, EOnlineDataAdvertisementType::ViaOnlineService);

	UE_LOG(LogTemp, Log, TEXT("[GameModeSelection] CreateSession room=%s, pw=%s, max=%d, map=%s"),
		*SessionData.RoomName,
		*SessionData.Password,
		SessionData.MaxPlayers,
		*SessionData.MapName);

	Sessions->CreateSession(0, SessionName, Settings);
}

void UGameModeSelection::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameModeSelection] Tạo session thất bại"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GameModeSelection] CreateSession thành công, chuyển sang LobbyMap"));
	UGameplayStatics::OpenLevel(this, TEXT("LobbyMap"), true, TEXT("?listen"));
}
