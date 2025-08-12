#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/Engine.h"
#include "LobbyWidget.generated.h"

class ULobbyPlayerSlot;
class UVerticalBox;
class UButton;
class UTextBlock;

UCLASS()
class HORRORGAME_API ULobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Update player slots when players join/leave
    UFUNCTION(BlueprintCallable)
    void UpdatePlayerSlots(const TArray<FString>& PlayerNames, const TArray<bool>& IsHostArray);

    // Called when a player joins the session
    UFUNCTION()
    void OnPlayerJoined(const FString& PlayerName, bool bIsHost);

    // Called when a player leaves the session
    UFUNCTION()
    void OnPlayerLeft(const FString& PlayerName);

protected:
    // UI Components
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* PlayerSlotsContainer;

    UPROPERTY(meta = (BindWidget))
    UButton* StartGameButton;

    UPROPERTY(meta = (BindWidget))
    UButton* InviteFriendsButton;

    UPROPERTY(meta = (BindWidget))
    UButton* LeaveButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LobbyStatusText;

    // Player Slot Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    TSubclassOf<ULobbyPlayerSlot> PlayerSlotClass;

    // Array to hold player slot widgets
    UPROPERTY()
    TArray<ULobbyPlayerSlot*> PlayerSlots;

    // Maximum number of players
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    int32 MaxPlayers = 4;

private:
    // Button event handlers
    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnInviteFriendsClicked();

    UFUNCTION()
    void OnLeaveClicked();

    // Session interface
    IOnlineSessionPtr SessionInterface;

    // Initialize session interface
    void InitializeSessionInterface();

    // Update lobby status text
    void UpdateLobbyStatus();

    // Check if local player is host
    bool IsLocalPlayerHost() const;
};