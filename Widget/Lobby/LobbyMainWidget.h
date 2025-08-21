// LobbyMainWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMainWidget.generated.h"

class ULobbyPlayerSlot;
class UInviteFriendsWidget;
class UFriendInviteWidget;
class ALobbyPlayerController;
struct FLobbySettings;
struct FLobbyPlayerData;
enum class ELobbyState : uint8;

UCLASS()
class HORRORGAME_API ULobbyMainWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Override NativeConstruct
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    /* ----------------------
       Widget bindings
       ---------------------- */

       // Header / info
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* LobbyNameText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* LobbyStateText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CountdownText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* PlayerCountText;

    // Player slots container
    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* PlayerSlotsContainer;

    // Buttons
    UPROPERTY(meta = (BindWidget))
    class UButton* ReadyButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* StartGameButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* LeaveLobbyButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* SaveSettingsButton;

    // Settings inputs
    UPROPERTY(meta = (BindWidget))
    class UEditableText* LobbyNameInput;

    UPROPERTY(meta = (BindWidget))
    class UEditableText* PasswordInput;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* PrivateLobbyCheckbox;

    UPROPERTY(meta = (BindWidget))
    class UComboBoxString* GameModeCombo;

    UPROPERTY(meta = (BindWidget))
    class UComboBoxString* MapCombo;

    /* ----------------------
       Widget classes (Blueprint-assigned)
       ---------------------- */

       // Class for player slot widget
    UPROPERTY(EditDefaultsOnly, Category = "Lobby|Widgets")
    TSubclassOf<ULobbyPlayerSlot> PlayerSlotWidgetClass;

    // Invite widget class used inside slot
    UPROPERTY(EditDefaultsOnly, Category = "Lobby|Widgets")
    TSubclassOf<UInviteFriendsWidget> InviteFriendWidgetClass;

    /* ----------------------
       Runtime state
       ---------------------- */

       // List of created slot widgets
    UPROPERTY()
    TArray<ULobbyPlayerSlot*> PlayerSlots;

    // Local flags
    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bIsReady;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bIsHost;

    // Current lobby state (Waiting, CountingDown, Starting, InGame, ...)
    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    ELobbyState CurrentState;

public:
    /* ----------------------
       Internal helpers (implementations in .cpp)
       ---------------------- */

    // Create the fixed number of player slots and attach invite widgets
    void CreatePlayerSlots();

    // Update lobby state text & behavior
    void UpdateLobbyState(ELobbyState NewState);

    // Update lobby UI from settings
    void UpdateLobbyInfo(const FLobbySettings& Settings);

    // Update slot UIs using array of players
    void UpdatePlayerSlots(const TArray<FLobbyPlayerData>& Players);

    // Update countdown display
    void UpdateCountdown(int32 SecondsLeft);

    // Show/hide host-only controls
    void SetHostControls(bool bInIsHost);

    // Update text on ready button
    void UpdateReadyButtonText();

    // Enable/disable start button
    void UpdateStartGameButtonState();

    // Initialize combo boxes and default settings UI
    void InitializeSettingsPanel();

    /* ----------------------
       Event handlers (bound to buttons / delegates)
       ---------------------- */

    UFUNCTION()
    void OnReadyButtonClicked();

    UFUNCTION()
    void OnStartGameButtonClicked();

    UFUNCTION()
    void OnLeaveLobbyButtonClicked();

    UFUNCTION()
    void OnSaveSettingsButtonClicked();

    // Called when clicking the invite control for a given slot
    UFUNCTION()
    void OnInviteSlotClicked(int32 SlotIndex);

    // Called when clicking leave/kick control for a slot
    UFUNCTION()
    void OnLeaveSlotClicked(int32 SlotIndex);
};
