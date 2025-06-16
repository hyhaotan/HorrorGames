#include "LobbyWidget.h"
#include "LobbySlotWidget.h"
#include "LobbyFriendListPopup.h"
#include "HorrorGame/Object/FriendEntryData.h"
#include "FriendListEntryWidget.h"
#include "Components/PanelWidget.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"

const FName ULobbyWidget::SessionName = FName(TEXT("GameSession"));
const FName ULobbyWidget::MapNameSettingKey = FName(TEXT("GameMap"));

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 1) Grab the Steam subsystem
	OnlineSubsystem = IOnlineSubsystem::Get(TEXT("Steam"));
	if (!OnlineSubsystem)
	{
		Log(TEXT("Could not find OnlineSubsystem Steam"), ELogVerbosity::Error);
		return;
	}

	FriendsInterface = OnlineSubsystem->GetFriendsInterface();
	SessionInterface = OnlineSubsystem->GetSessionInterface();
	IdentityInterface = OnlineSubsystem->GetIdentityInterface();

	// 2) Login to Steam if possible
	if (IdentityInterface.IsValid())
	{
		LoginCompleteHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
			0,
			FOnLoginCompleteDelegate::CreateUObject(this, &ULobbyWidget::OnLoginComplete)
		);
		IdentityInterface->Login(0, FOnlineAccountCredentials());
	}

	// 3) Bind session delegates
	if (SessionInterface.IsValid())
	{
		CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(this, &ULobbyWidget::OnCreateSessionComplete)
		);
		StartSessionCompleteHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
			FOnStartSessionCompleteDelegate::CreateUObject(this, &ULobbyWidget::OnStartSessionComplete)
		);
		JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
			FOnJoinSessionCompleteDelegate::CreateUObject(this, &ULobbyWidget::OnJoinSessionComplete)
		);
		InviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ULobbyWidget::OnSessionInviteAccepted)
		);

		// Khởi StartSession ngay (nếu session đã có)
		SessionInterface->StartSession(SessionName);
	}

	// 4) Initialize slots
	const int32 NumSlots = 4;
	LobbySlots.SetNum(NumSlots);
	LobbySlotWidgets.SetNum(NumSlots);

	FLobbySlotData LocalSlot;
	LocalSlot.bOccupied = true;
	if (IdentityInterface.IsValid())
	{
		TSharedPtr<const FUniqueNetId> LocalId = IdentityInterface->GetUniquePlayerId(0);
		if (LocalId.IsValid())
		{
			LocalSlot.PlayerId = FUniqueNetIdRepl(LocalId.ToSharedRef());
			LocalSlot.DisplayName = IdentityInterface->GetPlayerNickname(0);
		}
		else
		{
			LocalSlot.DisplayName = TEXT("LocalPlayer");
		}
	}
	else
	{
		LocalSlot.DisplayName = TEXT("LocalPlayer");
	}
	LobbySlots[0] = LocalSlot;

	for (int32 i = 1; i < NumSlots; ++i)
	{
		FLobbySlotData EmptySlot;
		EmptySlot.bOccupied = false;
		EmptySlot.DisplayName = TEXT("");
		LobbySlots[i] = EmptySlot;
	}

	// 5) Spawn slot widgets vào container
	if (!SlotsContainer)
	{
		Log(TEXT("SlotsContainer was not bound in UMG"), ELogVerbosity::Error);
		return;
	}

	for (int32 i = 0; i < NumSlots; ++i)
	{
		if (!LobbySlotWidgetClass)
		{
			Log(TEXT("LobbySlotWidgetClass not set in Blueprint"), ELogVerbosity::Error);
			continue;
		}

		ULobbySlotWidget* SlotWdg = CreateWidget<ULobbySlotWidget>(GetWorld(), LobbySlotWidgetClass);
		if (!SlotWdg) { continue; }

		if (LobbySlots[i].bOccupied)
		{
			SlotWdg->InitializeSlot(i, true, LobbySlots[i].DisplayName, LobbySlots[i].PlayerId);
		}
		else
		{
			SlotWdg->InitializeSlot(i, false);
		}

		SlotWdg->OnInviteClicked.AddDynamic(this, &ULobbyWidget::HandleOnSlotInviteClicked);
		LobbySlotWidgets[i] = SlotWdg;
		SlotsContainer->AddChild(SlotWdg);
	}
}

void ULobbyWidget::OnLoginComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (!bWasSuccessful)
	{
		Log(FString::Printf(TEXT("Steam Login failed: %s"), *Error), ELogVerbosity::Error);
		return;
	}
	Log(TEXT("Steam Login succeeded"));

	if (FriendsInterface.IsValid())
	{
		FriendsInterface->ReadFriendsList(
			LocalPlayerNum,
			TEXT("default"),
			FOnReadFriendsListComplete::CreateUObject(this, &ULobbyWidget::OnReadFriendsListComplete)
		);
	}
}

void ULobbyWidget::OnReadFriendsListComplete(int32 LocalPlayerNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (!bWasSuccessful)
	{
		Log(FString::Printf(TEXT("Failed to read friends: %s"), *ErrorStr), ELogVerbosity::Error);
		return;
	}
	Log(TEXT("ReadFriendsListComplete: friends have been loaded"));
	// Popup sẽ load dữ liệu khi cần.
}

void ULobbyWidget::CreatePartySession()
{
	if (!SessionInterface.IsValid()) return;

	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = 4;
	Settings.bIsLANMatch = false;
	Settings.bAllowJoinInProgress = true;
	Settings.bShouldAdvertise = true;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinViaPresence = true;

	// Custom: lưu map mặc định khi session bắt đầu
	Settings.Set(MapNameSettingKey, FString(TEXT("YourGameMapName")), EOnlineDataAdvertisementType::ViaOnlineService);

	Log(TEXT("Creating session (host)"));
	SessionInterface->CreateSession(0, SessionName, Settings);
}

void ULobbyWidget::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	Log(FString::Printf(TEXT("OnCreateSessionComplete: %s"), bWasSuccessful ? TEXT("true") : TEXT("false")), ELogVerbosity::Warning);

	if (!bWasSuccessful)
	{
		Log(TEXT("Tạo session thất bại"), ELogVerbosity::Error);
		return;
	}

	// Đọc custom MapName (nếu cần hiển lên UI)
	FString GameMapName;
	if (const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(InSessionName))
	{
		const FOnlineSessionSettings& S = NamedSession->SessionSettings;
		if (S.Get(MapNameSettingKey, GameMapName))
		{
			Log(FString::Printf(TEXT("Thông tin MapName lưu trong session: %s"), *GameMapName));
			// Lưu vào biến cục bộ nếu cần
		}
	}

	Log(TEXT("CreateSession thành công, chuyển sang LobbyMap (listen)"));
	UGameplayStatics::OpenLevel(this, TEXT("LobbyMap"), true, TEXT("?listen"));
}

void ULobbyWidget::OnStartSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		Log(TEXT("StartSession failed"), ELogVerbosity::Error);
		return;
	}
	Log(TEXT("StartSession succeeded (host), loading map"));

	if (const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(InSessionName))
	{
		const FOnlineSessionSettings& Settings = NamedSession->SessionSettings;
		FString GameMapName;
		if (Settings.Get(MapNameSettingKey, GameMapName))
		{
			UGameplayStatics::OpenLevel(GetWorld(), FName(*GameMapName), true);
		}
		else
		{
			Log(TEXT("MapNameSettingKey missing"), ELogVerbosity::Warning);
		}
	}
	else
	{
		Log(TEXT("Could not find session in OnStartSessionComplete"), ELogVerbosity::Error);
	}
}

void ULobbyWidget::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		Log(TEXT("JoinSession failed (client)"), ELogVerbosity::Error);
		return;
	}

	// ClientTravel đến connect string
	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(InSessionName, ConnectString) && !ConnectString.IsEmpty())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		Log(TEXT("ConnectString was empty in OnJoinSessionComplete"), ELogVerbosity::Error);
	}
}

void ULobbyWidget::OnSessionInviteAccepted(bool bWasInvited, int32 LocalPlayerNum, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasInvited || !InviteResult.IsValid())
	{
		Log(TEXT("Invite acceptance failed or InviteResult invalid"), ELogVerbosity::Warning);
		return;
	}

	SessionInterface->JoinSession(LocalPlayerNum, SessionName, InviteResult);
	Log(FString::Printf(TEXT("Client joining session invited by %s"), *UserId->ToString()));
}

void ULobbyWidget::HandleOnSlotInviteClicked(int32 SlotIndex)
{
	if (!LobbyFriendListPopupClass)
	{
		Log(TEXT("LobbyFriendListPopupClass not set"), ELogVerbosity::Error);
		return;
	}

	if (CurrentFriendListPopup)
	{
		CurrentFriendListPopup->RemoveFromParent();
		CurrentFriendListPopup = nullptr;
	}

	CurrentFriendListPopup = CreateWidget<ULobbyFriendListPopup>(GetWorld(), LobbyFriendListPopupClass);
	if (!CurrentFriendListPopup) return;

	CurrentFriendListPopup->InitializePopup(this, SlotIndex);
	CurrentFriendListPopup->AddToViewport();
}

void ULobbyWidget::InviteFriendToSlot(const FUniqueNetIdRepl& FriendId, int32 SlotIndex)
{
	if (!SessionInterface.IsValid())
	{
		Log(TEXT("SessionInterface invalid in InviteFriendToSlot"), ELogVerbosity::Error);
		return;
	}
	if (SlotIndex < 0 || SlotIndex >= LobbySlots.Num())
	{
		Log(TEXT("SlotIndex invalid in InviteFriendToSlot"), ELogVerbosity::Error);
		return;
	}

	bool bSent = SessionInterface->SendSessionInviteToFriend(0, SessionName, *FriendId);
	if (bSent)
	{
		LobbySlots[SlotIndex].bOccupied = false;
		LobbySlots[SlotIndex].PlayerId = FriendId;
		LobbySlots[SlotIndex].DisplayName = TEXT("Invited...");
		UpdateSlotWidget(SlotIndex);

		FString InviteeIdString = FriendId.IsValid() ? FriendId->ToDebugString() : TEXT("UnknownID");
		Log(FString::Printf(TEXT("Sent invite to %s into slot %d"), *InviteeIdString, SlotIndex));
	}
	else
	{
		Log(TEXT("SendSessionInviteToFriend failed"), ELogVerbosity::Error);
	}

	if (CurrentFriendListPopup)
	{
		CurrentFriendListPopup->RemoveFromParent();
		CurrentFriendListPopup = nullptr;
	}
}

void ULobbyWidget::UpdateSlotWidget(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= LobbySlotWidgets.Num()) return;

	ULobbySlotWidget* SlotWdg = LobbySlotWidgets[SlotIndex];
	if (!SlotWdg) return;

	const FLobbySlotData& Data = LobbySlots[SlotIndex];
	if (Data.bOccupied)
	{
		SlotWdg->InitializeSlot(SlotIndex, true, Data.DisplayName, Data.PlayerId);
	}
	else
	{
		SlotWdg->InitializeSlot(SlotIndex, false);
	}
}

void ULobbyWidget::Log(const FString& Msg, ELogVerbosity::Type Verbosity)
{
	FMsg::Logf(__FILE__, __LINE__, LogTemp.GetCategoryName(), Verbosity, TEXT("[Lobby] %s"), *Msg);
}
