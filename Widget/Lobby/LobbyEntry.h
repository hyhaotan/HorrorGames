#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyEntry.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyJoinRequested, const FString&, LobbyID);

UCLASS()
class HORRORGAME_API ULobbyEntry : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(BlueprintAssignable)
    FOnLobbyJoinRequested OnLobbyJoinRequested;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LobbyNameText;

    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton;

    // Set lobby data
    void SetLobbyData(const FString& InLobbyData);

protected:
    UFUNCTION()
    void OnJoinButtonClicked();

private:
    FString LobbyData;
};