#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyWidget.generated.h"

UCLASS()
class HORRORGAME_API ULobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    void UpdatePlayerSlots(const TArray<FUniqueNetIdRepl>& PlayerIds);

protected:
    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* PlayerSlotsContainer;

    UPROPERTY(meta = (BindWidget))
    class UButton* StartGameButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* InviteFriendsButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* LeaveButton;

    UPROPERTY(EditDefaultsOnly, Category = "Lobby")
    TSubclassOf<class ULobbyPlayerSlot> PlayerSlotClass;

    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnInviteFriendsClicked();

    UFUNCTION()
    void OnLeaveClicked();

private:
    IOnlineSessionPtr SessionInterface;
    TArray<class ULobbyPlayerSlot*> PlayerSlots;

    void InitializeSessionInterface();
};