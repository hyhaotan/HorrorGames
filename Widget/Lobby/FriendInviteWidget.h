#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FriendInviteWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UEditableText;

USTRUCT(BlueprintType)
struct FSteamFriendInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString SteamID;

    UPROPERTY(BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite)
    bool bIsOnline;

    UPROPERTY(BlueprintReadWrite)
    bool bIsInGame;

    FSteamFriendInfo()
    {
        bIsOnline = false;
        bIsInGame = false;
    }
};

UCLASS()
class HORRORGAME_API UFriendInviteWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* FriendsListBox;

    UPROPERTY(meta = (BindWidget))
    UButton* RefreshButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    UPROPERTY(meta = (BindWidget))
    UEditableText* PlayerIDSearchBox;

    UPROPERTY(meta = (BindWidget))
    UButton* SearchPlayerButton;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> FriendItemWidgetClass;

    UFUNCTION(BlueprintCallable)
    void RefreshFriendsList();

    UFUNCTION(BlueprintCallable)
    void SearchPlayerByID(const FString& PlayerID);

    UFUNCTION(BlueprintImplementableEvent)
    void OnFriendInvited(const FString& FriendName);

protected:
    UFUNCTION()
    void OnRefreshClicked();

    UFUNCTION()
    void OnCloseClicked();

    UFUNCTION()
    void OnSearchPlayerClicked();

    void PopulateFriendsList(const TArray<FSteamFriendInfo>& Friends);
    void AddFriendToList(const FSteamFriendInfo& FriendInfo);

private:
    TArray<FSteamFriendInfo> CachedFriends;
};