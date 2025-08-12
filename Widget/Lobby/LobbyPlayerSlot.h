#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "LobbyPlayerSlot.generated.h"

UCLASS()
class HORRORGAME_API ULobbyPlayerSlot : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Set player information for occupied slot
    UFUNCTION(BlueprintCallable)
    void SetPlayerInfo(const FString& PlayerName, bool bInIsHost);

    // Set slot as empty
    UFUNCTION(BlueprintCallable)
    void SetEmpty();

    // Check if slot is occupied
    UFUNCTION(BlueprintCallable)
    bool IsOccupied() const { return bIsOccupied; }

    // Check if this slot's player is host
    UFUNCTION(BlueprintCallable)
    bool IsHost() const { return bIsHost; }

    // Get player name in this slot
    UFUNCTION(BlueprintCallable)
    FString GetPlayerName() const { return CurrentPlayerName; }

protected:
    // UI Components
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget))
    UButton* InviteButton;

    UPROPERTY(meta = (BindWidget))
    UButton* KickButton;

    UPROPERTY(meta = (BindWidget))
    UImage* PlayerAvatar;

    UPROPERTY(meta = (BindWidget))
    UImage* HostIcon;

private:
    // Button event handlers
    UFUNCTION()
    void OnInviteButtonClicked();

    UFUNCTION()
    void OnKickButtonClicked();

    // Slot state
    bool bIsOccupied = false;
    bool bIsHost = false;
    FString CurrentPlayerName;

    // Update button visibility based on slot state and permissions
    void UpdateButtonVisibility();
};