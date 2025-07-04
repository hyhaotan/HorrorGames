#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "HorrorGame/Data/SessionSettingsData.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ServerBrowserWidget.generated.h"

class UScrollBox;
class UEditableTextBox;
class UButton;
class USessionRowWidget;
class UGameModeSelection;
class UCreateSessionWidget;

UCLASS()
class HORRORGAME_API UServerBrowserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Constants
    static const FName Name_GameSession;
    static const FName RoomKey;
    static const FName PasswordKey;
    static const FName MapKey;

protected:
    // UI Bindings
    UPROPERTY(meta = (BindWidget)) UButton* RefreshButton;
    UPROPERTY(meta = (BindWidget)) UButton* BackButton;
    UPROPERTY(meta = (BindWidget)) UButton* CreateLobbyButton;
    UPROPERTY(meta = (BindWidget)) UScrollBox* SessionListBox;
    UPROPERTY(meta = (BindWidget)) UEditableTextBox* SearchBox;

    // Session search
    TSharedPtr<FOnlineSessionSearch> SearchSettings;
    FDelegateHandle FindHandle;
    FDelegateHandle JoinHandle;

    // Session row class
    UPROPERTY(EditAnywhere, Category = "Session") TSubclassOf<USessionRowWidget> SessionRowClass;
    UPROPERTY(EditAnywhere, Category = "Session") TSubclassOf<UGameModeSelection> GameModeSelectionClass;

    UPROPERTY(EditAnywhere, Category = "Session") TSubclassOf<UCreateSessionWidget> CreateSessionWidgetClass;

    // Handlers
	UFUNCTION()
    void OnRefreshClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnCreateLobbyClicked();

    void FindSessions(const FString& Filter = TEXT(""));
    void OnFindSessionsComplete(bool bSuccess);
    void JoinSession(int32 Index);
    void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);

    IOnlineSessionPtr GetSessionInterface() const;
};