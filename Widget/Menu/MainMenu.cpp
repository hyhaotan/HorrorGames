#include "MainMenu.h"
#include "HorrorGame/GameInstance/SteamLobbySubsystem.h"
#include "HorrorGame/Object/PlayerIDManager.h"
#include "HorrorGame/Widget/Lobby/LobbyEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"

void UMainMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize state variables
    bIsSearchingLobbies = false;
    bIsCreatingLobby = false;
    bIsJoiningLobby = false;

    // Get Steam Lobby Subsystem
    SteamLobbySubsystem = GetGameInstance()->GetSubsystem<USteamLobbySubsystem>();

    if (SteamLobbySubsystem)
    {
        // Bind Steam Lobby events
        SteamLobbySubsystem->OnLobbyCreated.AddDynamic(this, &UMainMenu::OnLobbyCreated);
        SteamLobbySubsystem->OnLobbyJoined.AddDynamic(this, &UMainMenu::OnLobbyJoined);
        SteamLobbySubsystem->OnLobbySearchComplete.AddDynamic(this, &UMainMenu::OnLobbySearchComplete);
    }
    else
    {
        UpdateStatusMessage(TEXT("Failed to initialize Steam Lobby System"), true);
        return;
    }

    // Bind button events
    if (CreateLobbyButton)
    {
        CreateLobbyButton->OnClicked.AddDynamic(this, &UMainMenu::OnCreateLobbyClicked);
    }

    if (JoinLobbyButton)
    {
        JoinLobbyButton->OnClicked.AddDynamic(this, &UMainMenu::OnJoinLobbyClicked);
    }

    if (RefreshLobbiesButton)
    {
        RefreshLobbiesButton->OnClicked.AddDynamic(this, &UMainMenu::OnRefreshLobbiesClicked);
    }

    if (ExitGameButton)
    {
        ExitGameButton->OnClicked.AddDynamic(this, &UMainMenu::OnExitGameClicked);
    }

    // Initialize UI state
    if (BorderListBox)
    {
        BorderListBox->SetVisibility(ESlateVisibility::Hidden);
    }

    InitializePlayerName();
    UpdateStatusMessage(TEXT("Ready to play!"));

    // Auto-refresh lobbies on start
    OnRefreshLobbiesClicked();
}

void UMainMenu::NativeDestruct()
{
    // Clean up delegates
    if (SteamLobbySubsystem)
    {
        SteamLobbySubsystem->OnLobbyCreated.RemoveDynamic(this, &UMainMenu::OnLobbyCreated);
        SteamLobbySubsystem->OnLobbyJoined.RemoveDynamic(this, &UMainMenu::OnLobbyJoined);
        SteamLobbySubsystem->OnLobbySearchComplete.RemoveDynamic(this, &UMainMenu::OnLobbySearchComplete);
    }

    Super::NativeDestruct();
}

void UMainMenu::InitializePlayerName()
{
    if (!PlayerNameText) return;

    // Create temporary PlayerIDManager to get Steam name
    UPlayerIDManager* TempManager = NewObject<UPlayerIDManager>(this);
    if (IsValid(TempManager))
    {
        FString SteamName = TempManager->GetCurrentSteamName();
        if (!SteamName.IsEmpty())
        {
            PlayerNameText->SetText(FText::FromString(FString::Printf(TEXT("Welcome, %s"), *SteamName)));
        }
        else
        {
            PlayerNameText->SetText(FText::FromString(TEXT("Welcome, Player")));
        }
    }
    else
    {
        PlayerNameText->SetText(FText::FromString(TEXT("Welcome, Player")));
    }
}

void UMainMenu::OnCreateLobbyClicked()
{
    if (!SteamLobbySubsystem || bIsCreatingLobby) return;

    // Validate lobby name input
    FString LobbyName;
    if (LobbyNameInput && !LobbyNameInput->GetText().IsEmpty())
    {
        LobbyName = LobbyNameInput->GetText().ToString().TrimStartAndEnd();
        if (LobbyName.Len() > 50) // Limit lobby name length
        {
            UpdateStatusMessage(TEXT("Lobby name too long (max 50 characters)"), true);
            return;
        }
    }
    else
    {
        LobbyName = TEXT("Horror Game Lobby");
    }

    FLobbySettings Settings;
    Settings.LobbyName = LobbyName;
    Settings.MaxPlayers = 4;
    Settings.bIsPrivate = false;
    Settings.GameMode = TEXT("Classic");
    Settings.MapName = TEXT("Abandoned Hospital");

    bIsCreatingLobby = true;
    SteamLobbySubsystem->CreateLobby(Settings);

    UpdateStatusMessage(TEXT("Creating lobby..."));
    SetButtonsEnabled(false);
}

void UMainMenu::OnJoinLobbyClicked()
{
    if (!SteamLobbySubsystem) return;

    // Toggle lobby list visibility
    if (BorderListBox)
    {
        if (BorderListBox->GetVisibility() == ESlateVisibility::Hidden)
        {
            BorderListBox->SetVisibility(ESlateVisibility::Visible);

            // Refresh lobbies when showing the list
            if (!bIsSearchingLobbies)
            {
                OnRefreshLobbiesClicked();
            }
        }
        else
        {
            BorderListBox->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UMainMenu::JoinSpecificLobby(const FString& LobbyID)
{
    if (!SteamLobbySubsystem || bIsJoiningLobby) return;

    if (LobbyID.IsEmpty())
    {
        UpdateStatusMessage(TEXT("Invalid Lobby ID"), true);
        return;
    }

    bIsJoiningLobby = true;
    SteamLobbySubsystem->JoinLobby(LobbyID);

    UpdateStatusMessage(TEXT("Joining lobby..."));
    SetButtonsEnabled(false);

    // Hide lobby list
    if (BorderListBox)
    {
        BorderListBox->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UMainMenu::OnRefreshLobbiesClicked()
{
    if (!SteamLobbySubsystem || bIsSearchingLobbies) return;

    bIsSearchingLobbies = true;
    SteamLobbySubsystem->SearchLobbies();

    UpdateStatusMessage(TEXT("Searching for lobbies..."));

    if (RefreshLobbiesButton)
    {
        RefreshLobbiesButton->SetIsEnabled(false);
    }

    // Clear existing lobbies list
    ClearLobbiesList();
}

void UMainMenu::OnExitGameClicked()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UMainMenu::OnLobbyCreated(bool bSuccess)
{
    bIsCreatingLobby = false;
    SetButtonsEnabled(true);

    if (bSuccess)
    {
        UpdateStatusMessage(TEXT("Lobby created successfully! Loading lobby..."));
        UE_LOG(LogTemp, Log, TEXT("Lobby created successfully"));

        UGameplayStatics::OpenLevel(GetWorld(), TEXT("LobbyMap"));
    }
    else
    {
        UpdateStatusMessage(TEXT("Failed to create lobby. Please try again."), true);
        UE_LOG(LogTemp, Warning, TEXT("Failed to create lobby"));
    }
}

void UMainMenu::OnLobbyJoined(bool bSuccess)
{
    bIsJoiningLobby = false;
    SetButtonsEnabled(true);

    if (bSuccess)
    {
        UpdateStatusMessage(TEXT("Successfully joined lobby! Loading lobby..."));
        UE_LOG(LogTemp, Log, TEXT("Joined lobby successfully"));

        UGameplayStatics::OpenLevel(GetWorld(), TEXT("LobbyMap"));
    }
    else
    {
        UpdateStatusMessage(TEXT("Failed to join lobby. It may be full or no longer available."), true);
        UE_LOG(LogTemp, Warning, TEXT("Failed to join lobby"));
    }
}

void UMainMenu::OnLobbySearchComplete(bool bSuccess, const TArray<FString>& LobbyList)
{
    bIsSearchingLobbies = false;

    if (RefreshLobbiesButton)
    {
        RefreshLobbiesButton->SetIsEnabled(true);
    }

    if (bSuccess)
    {
        if (LobbyList.Num() > 0)
        {
            PopulateLobbiesList(LobbyList);
            UpdateStatusMessage(FString::Printf(TEXT("Found %d lobbies"), LobbyList.Num()));
        }
        else
        {
            UpdateStatusMessage(TEXT("No lobbies found. Create one to get started!"));
        }
    }
    else
    {
        UpdateStatusMessage(TEXT("Failed to search for lobbies. Check your connection."), true);
        UE_LOG(LogTemp, Warning, TEXT("Lobby search failed"));
    }
}

void UMainMenu::OnSpecificLobbyJoinClicked(const FString& LobbyID)
{
    JoinSpecificLobby(LobbyID);
}

void UMainMenu::PopulateLobbiesList(const TArray<FString>& Lobbies)
{
    if (!LobbiesListBox || !LobbyEntryWidgetClass) return;

    ClearLobbiesList();

    for (const FString& LobbyData : Lobbies)
    {
        if (LobbyData.IsEmpty()) continue;

        // Create lobby entry widget
        ULobbyEntry* LobbyEntry = CreateWidget<ULobbyEntry>(this, LobbyEntryWidgetClass);
        if (LobbyEntry)
        {
            LobbyEntry->SetLobbyData(LobbyData);
            LobbyEntry->OnLobbyJoinRequested.AddDynamic(this, &UMainMenu::OnLobbyEntryJoinRequested);
            LobbiesListBox->AddChild(LobbyEntry);
        }
    }

    // Show lobby list if we have lobbies
    if (Lobbies.Num() > 0 && BorderListBox)
    {
        BorderListBox->SetVisibility(ESlateVisibility::Visible);
    }
}

void UMainMenu::OnLobbyEntryJoinRequested(const FString& LobbyID)
{
    JoinSpecificLobby(LobbyID);
}

void UMainMenu::UpdateStatusMessage(const FString& Message, bool bIsError)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
        // You could set different colors for error vs normal messages here
    }

    // Also show debug message if it's an error
    if (bIsError && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, Message);
    }
}

void UMainMenu::SetButtonsEnabled(bool bEnabled)
{
    if (CreateLobbyButton)
    {
        CreateLobbyButton->SetIsEnabled(bEnabled);
    }

    if (JoinLobbyButton)
    {
        JoinLobbyButton->SetIsEnabled(bEnabled);
    }

    if (RefreshLobbiesButton && !bIsSearchingLobbies)
    {
        RefreshLobbiesButton->SetIsEnabled(bEnabled);
    }
}

void UMainMenu::ClearLobbiesList()
{
    if (LobbiesListBox)
    {
        LobbiesListBox->ClearChildren();
    }
}