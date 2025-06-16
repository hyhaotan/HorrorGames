#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ServerBrowserWidget.generated.h"

class UButton;
class UScrollBox;
class UTextBlock;
class USessionRowWidget;
class UCreateSessionWidget;
class UEditableTextBox;
class UGameModeSelection;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionDelegate, int32, SessionIndex);

UCLASS()
class HORRORGAME_API UServerBrowserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    /** Class của CreateSessionWidget (được truyền từ GameModeSelection) */
    UPROPERTY(EditAnywhere, Category = "ServerBrowser")
    TSubclassOf<UCreateSessionWidget> CreateSessionWidgetClass;

    /** Class của SessionRowWidget để tạo từng row hiển thị kết quả */
    UPROPERTY(EditAnywhere, Category = "ServerBrowser")
    TSubclassOf<USessionRowWidget> SessionRowWidgetClass;
  
    UPROPERTY(EditAnywhere, Category = "ServerBrowser")
    TSubclassOf<UGameModeSelection> GameModeSelectionClass;

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* RefreshButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CreateLobbyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SearchButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* FindSessionTextBox;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* SessionListScrollBox;

    /** Delegate và handle cho FindSessions */
    FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
    FDelegateHandle OnFindSessionsCompleteDelegateHandle;

    /** Delegate và handle cho JoinSession */
    FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
    FDelegateHandle OnJoinSessionCompleteDelegateHandle;

    /** Delegate và handle cho CreateSession (khi host) */
    FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
    FDelegateHandle OnCreateSessionCompleteDelegateHandle;

    /** Search object */
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    /** Lấy interface của OnlineSession */
    IOnlineSessionPtr GetSessionInterface() const;

    /** Hàm gọi để tìm session */
    UFUNCTION()
    void FindSessions();

    UFUNCTION()
    void OnRefreshClicked();

    UFUNCTION()
	void OnBackWidget();

    /** Callback khi FindSessions hoàn thành */
    void OnFindSessionsComplete(bool bWasSuccessful);

    /** Hàm được gọi khi user nhấn join ở 1 row (được bind delegate từ row) */
    UFUNCTION()
    void JoinSessionByIndex(int32 Index);

    /** Callback khi JoinSession hoàn thành */
    void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);

    /** Khi user nhấn Create Lobby */
    UFUNCTION()
    void OnCreateLobbyClicked();

    /** Callback khi CreateSession hoàn thành */
    void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);

    /** Hàm thực sự tạo session (sao chép logic từ GameModeSelection::StartMultiplayer) */
    void CreateNewSession(const FSessionSettingsData& SessionData);

    /** Lắng nghe từ CreateSessionWidget */
    UFUNCTION()
    void OnCreateSessionRequest(const FSessionSettingsData& SessionData);
};
