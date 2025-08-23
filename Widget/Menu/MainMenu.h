#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Data/LobbyType.h"
#include "MainMenu.generated.h"

class UButton;
class UTextBlock;
class UEditableText;
class UScrollBox;
class UBorder;
class UHorizontalBox;
class USteamLobbySubsystem;
class ULobbyEntry;
class UConfirmExitWidget;
class UCreditsWidget;

UCLASS()
class HORRORGAME_API UMainMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // UI Components
    UPROPERTY(meta = (BindWidget))
    UButton* CreateLobbyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CreditsLobbyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* JoinLobbyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* RefreshLobbiesButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ExitGameButton;

    UPROPERTY(meta = (BindWidget))
    UBorder* BorderListBox;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* LobbiesListBox;

    UPROPERTY(meta = (BindWidget))
    UEditableText* LobbyNameInput;

    UPROPERTY(meta = (BindWidget))
    UEditableText* JoinLobbyIDInput;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusText;

protected:
    // Button handlers
    UFUNCTION()
    void OnCreateLobbyClicked();

    UFUNCTION()
    void OnJoinLobbyClicked();

    UFUNCTION()
    void OnCreditsLobbyClicked();

    UFUNCTION()
    void OnRefreshLobbiesClicked();

    UFUNCTION()
    void OnExitGameClicked();

    // Lobby management
    UFUNCTION()
    void JoinSpecificLobby(const FString& LobbyID);

    UFUNCTION()
    void OnSpecificLobbyJoinClicked(const FString& LobbyID);

    // Event handlers for Steam Lobby Subsystem
    UFUNCTION()
    void OnLobbyCreated(bool bSuccess);

    UFUNCTION()
    void OnLobbyJoined(bool bSuccess);

    UFUNCTION()
    void OnLobbySearchComplete(bool bSuccess, const TArray<FString>& LobbyList);

    // Helper functions
    void InitializePlayerName();
    void PopulateLobbiesList(const TArray<FString>& Lobbies);
    void UpdateStatusMessage(const FString& Message, bool bIsError = false);
    void SetButtonsEnabled(bool bEnabled);
    void ClearLobbiesList();

    // Event handler for lobby entry
    UFUNCTION()
    void OnLobbyEntryJoinRequested(const FString& LobbyID);

public:
    // Class reference for lobby entry widget
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<ULobbyEntry> LobbyEntryWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UConfirmExitWidget> ConfirmExitWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UCreditsWidget> CreditsWidgetClass;

private:
    UPROPERTY()
    USteamLobbySubsystem* SteamLobbySubsystem;

    // State tracking
    bool bIsSearchingLobbies;
    bool bIsCreatingLobby;
    bool bIsJoiningLobby;
};