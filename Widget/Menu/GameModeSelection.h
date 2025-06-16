#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Data/SessionSettingsData.h"
#include "GameModeSelection.generated.h"

// Forward declarations
class UButton;
class UWidgetAnimation;
class UMainMenu;
class UServerBrowserWidget;
class UCreateSessionWidget;

UCLASS()
class HORRORGAME_API UGameModeSelection : public UUserWidget
{
	GENERATED_BODY()

public:
	// Gọi khi widget đươc tạo xong
	virtual void NativeConstruct() override;

	static const FName SessionName;
	static const FName RoomNameKey;
	static const FName PasswordKey;
	static const FName MapNameKey;

	void ShowAnimGameMode() { PlayAnimation(ShowAnim); }
	void HideAnimGameMode() { PlayAnimation(HideAnim); }

protected:
	/** Single Player button */
	UPROPERTY(meta = (BindWidget))
	UButton* SinglePlayerButton;

	/** Multiplayer button */
	UPROPERTY(meta = (BindWidget))
	UButton* MultiplayerButton;

	/** Back button */
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HideAnim;

	/** Class của MainMenu để về lại */
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UMainMenu> MainMenuClass;

	/** Class của ServerBrowserWidget để show danh sách server */
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UServerBrowserWidget> ServerBrowserWidgetClass;

	/** Class của CreateSessionWidget để chuyển sang tạo lobby nếu user muốn tự host */
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UCreateSessionWidget> CreateSessionWidgetClass;

	/** Instance hiện đang hiển CreateSessionWidget (nếu có) */
	UPROPERTY()
	UCreateSessionWidget* ActiveCreateSessionWidget = nullptr;

	/** Trạng thái menu hiện tại */
	enum class EGameModeState : uint8
	{
		None,
		Single,
		Multiplayer,
		Back
	};
	EGameModeState CurrentState;

	/** Tên session (static) */
	static const FName SessionName;

	/** Custom key RoomName */
	static const FName RoomNameKey;

	/** Custom key Password */
	static const FName PasswordKey;

	/** Custom key MapName */
	static const FName MapNameKey;

	// =============================
	// ===    UFUNCTIONs       ===
	// =============================

	/** Khi user bấm Single Player */
	UFUNCTION()
	void OnSinglePlayerButtonClicked();

	/** Khi user bấm Multiplayer */
	UFUNCTION()
	void OnMultiplayerButtonClicked();

	/** Khi user bấm Back */
	UFUNCTION()
	void OnBackButtonClicked();

	/** Callback khi HideAnim play xong */
	UFUNCTION()
	void OnHideAnimationFinished();

	/** Lắng nghe thông báo user bấm “Create” từ CreateSessionWidget */
	UFUNCTION()
	void OnCreateSessionRequest(const FSessionSettingsData& SessionData);

	/** Callback khi CreateSession API trả về */
	UFUNCTION()
	void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);

	/** Thực sự gọi OpenLevel cho SinglePlayer */
	void StartSinglePlayer();

	/** Tạo session với dữ liệu do user nhập */
	void StartMultiplayer(const FSessionSettingsData& SessionData);
};
