#include "LobbyWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "LobbyPlayerSlot.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize session interface
    InitializeSessionInterface();

    // Bind button clicks
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

    // Initialize player slots
    if (PlayerSlotsContainer && PlayerSlotClass)
    {
        ULobbyPlayerSlot* PlayerSlotWidgetPtr = nullptr;
        for (int32 i = 0; i < 4; i++)
        {
            PlayerSlotWidgetPtr = CreateWidget<ULobbyPlayerSlot>(this, PlayerSlotClass);
            if (PlayerSlotWidgetPtr)
            {
                PlayerSlotsContainer->AddChild(PlayerSlotWidgetPtr);
                PlayerSlots.Add(PlayerSlotWidgetPtr);
                PlayerSlotWidgetPtr->SetEmpty();
            }
        }
    }
}

void ULobbyWidget::InitializeSessionInterface()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(TEXT("Steam"));
    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();
    }
}

void ULobbyWidget::OnStartGameClicked()
{
    if (SessionInterface.IsValid())
    {
        // Start the game only if we're the host
        if (SessionInterface->GetNamedSession(NAME_GameSession))
        {
            UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"), true, TEXT("listen"));
        }
    }
}

void ULobbyWidget::OnInviteFriendsClicked()
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

void ULobbyWidget::OnLeaveClicked()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->DestroySession(NAME_GameSession);
        UGameplayStatics::OpenLevel(this, TEXT("MainMenu"), true);
    }
}

void ULobbyWidget::UpdatePlayerSlots(const TArray<FUniqueNetIdRepl>& PlayerIds)
{
    for (int32 i = 0; i < PlayerSlots.Num(); i++)
    {
        if (i < PlayerIds.Num())
        {
            bool bIsHost = (i == 0); // First player is always the host
            PlayerSlots[i]->SetPlayerInfo(PlayerIds[i]->ToString(), bIsHost);
        }
        else
        {
            PlayerSlots[i]->SetEmpty();
        }
    }
}