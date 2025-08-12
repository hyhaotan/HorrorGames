#include "LobbyWidget.h"
#include "LobbyPlayerSlot.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"

void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize session interface with fallback
    InitializeSessionInterface();

    // Bind button events
    if (StartGameButton)
    {
        StartGameButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartGameClicked);
    }

    if (InviteFriendsButton)
    {
        InviteFriendsButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnInviteFriendsClicked);
    }

    if (LeaveButton)
    {
        LeaveButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnLeaveClicked);
    }

    // Create player slots
    if (PlayerSlotsContainer && PlayerSlotClass)
    {
        PlayerSlots.Empty();
        PlayerSlotsContainer->ClearChildren();

        for (int32 i = 0; i < MaxPlayers; i++)
        {
            ULobbyPlayerSlot* NewSlot = CreateWidget<ULobbyPlayerSlot>(this, PlayerSlotClass);
            if (NewSlot)
            {
                PlayerSlotsContainer->AddChild(NewSlot);
                PlayerSlots.Add(NewSlot);
                NewSlot->SetEmpty();
            }
        }
    }

    UpdateLobbyStatus();
}

void ULobbyWidget::InitializeSessionInterface()
{
    // Try to get any available online subsystem
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        UE_LOG(LogTemp, Log, TEXT("Using Online Subsystem: %s"),
            *OnlineSubsystem->GetSubsystemName().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Online Subsystem available - running in offline mode"));
    }
}

void ULobbyWidget::UpdatePlayerSlots(const TArray<FString>& PlayerNames, const TArray<bool>& IsHostArray)
{
    if (PlayerNames.Num() != IsHostArray.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerNames and IsHostArray size mismatch"));
        return;
    }

    // Update each slot
    for (int32 i = 0; i < PlayerSlots.Num(); i++)
    {
        if (i < PlayerNames.Num() && PlayerSlots[i])
        {
            PlayerSlots[i]->SetPlayerInfo(PlayerNames[i], IsHostArray[i]);
        }
        else if (PlayerSlots[i])
        {
            PlayerSlots[i]->SetEmpty();
        }
    }

    UpdateLobbyStatus();
}

void ULobbyWidget::OnPlayerJoined(const FString& PlayerName, bool bIsHost)
{
    // Find first empty slot and assign player
    for (ULobbyPlayerSlot* Slotes : PlayerSlots)
    {
        if (Slotes && !Slotes->IsOccupied())
        {
            Slotes->SetPlayerInfo(PlayerName, bIsHost);
            break;
        }
    }

    UpdateLobbyStatus();
}

void ULobbyWidget::OnPlayerLeft(const FString& PlayerName)
{
    // Find player's slot and set it empty
    for (ULobbyPlayerSlot* Slots : PlayerSlots)
    {
        if (Slots && Slots->IsOccupied() && Slots->GetPlayerName() == PlayerName)
        {
            Slots->SetEmpty();
            break;
        }
    }

    UpdateLobbyStatus();
}

void ULobbyWidget::OnStartGameClicked()
{
    // Check if we can start (host only in online mode, anyone in offline)
    bool bCanStart = true;

    if (SessionInterface.IsValid())
    {
        bCanStart = IsLocalPlayerHost();
    }

    if (!bCanStart)
    {
        UE_LOG(LogTemp, Warning, TEXT("Only host can start the game"));
        return;
    }

    // Start the game
    UE_LOG(LogTemp, Log, TEXT("Starting game from lobby"));
    UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"), true, TEXT("listen"));
}

void ULobbyWidget::OnInviteFriendsClicked()
{
    // Try Steam first, then fallback to generic invite
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

    if (OnlineSubsystem)
    {
        FName SubsystemName = OnlineSubsystem->GetSubsystemName();

        if (SubsystemName == TEXT("Steam"))
        {
            // Use Steam overlay
            IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
            if (ExternalUI.IsValid())
            {
                ExternalUI->ShowFriendsUI(0);
                return;
            }
        }
        else if (SubsystemName == TEXT("NULL"))
        {
            // Offline mode - show message
            UE_LOG(LogTemp, Log, TEXT("Running in offline mode - invite functionality disabled"));
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Invite friends not available"));
}

void ULobbyWidget::OnLeaveClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Leaving lobby"));

    if (SessionInterface.IsValid())
    {
        // Destroy session if host
        if (IsLocalPlayerHost())
        {
            SessionInterface->DestroySession(NAME_GameSession);
        }
    }

    // Return to main menu regardless
    UGameplayStatics::OpenLevel(this, TEXT("MainMenu"), true);
}

void ULobbyWidget::UpdateLobbyStatus()
{
    if (!LobbyStatusText) return;

    int32 CurrentPlayers = 0;
    for (const ULobbyPlayerSlot* Slotss : PlayerSlots)
    {
        if (Slotss && Slotss->IsOccupied())
        {
            CurrentPlayers++;
        }
    }

    FString StatusText = FString::Printf(TEXT("Lobby: %d/%d Players"), CurrentPlayers, MaxPlayers);
    LobbyStatusText->SetText(FText::FromString(StatusText));

    // Update start button state (only host can start)
    if (StartGameButton)
    {
        bool bCanStart = IsLocalPlayerHost() && CurrentPlayers >= 1; // Minimum 1 player to start
        StartGameButton->SetIsEnabled(bCanStart);
    }
}

bool ULobbyWidget::IsLocalPlayerHost() const
{
    if (!SessionInterface.IsValid())
    {
        // In offline mode, treat first player as host
        return true;
    }

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    if (Session)
    {
        APlayerController* PC = GetOwningPlayer();
        if (PC && PC->PlayerState)
        {
            return Session->OwningUserId == PC->PlayerState->GetUniqueId();
        }
    }

    return false;
}