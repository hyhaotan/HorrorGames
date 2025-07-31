#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayerSlot.generated.h"

UCLASS()
class HORRORGAME_API ULobbyPlayerSlot : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetPlayerInfo(const FString& PlayerName, bool bIsHost);
    void SetEmpty();
    bool IsOccupied() const { return bIsOccupied; }

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget))
    class UImage* PlayerIcon;

    UPROPERTY(meta = (BindWidget))
    class UButton* InviteButton;

    UFUNCTION()
    void OnInviteButtonClicked();

private:
    bool bIsOccupied = false;
    bool bIsHost = false;

    UFUNCTION()
    void HandleInviteButtonVisibility();
};