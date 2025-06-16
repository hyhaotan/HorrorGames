#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Data/SessionSettingsData.h" 
#include "CreateSessionWidget.generated.h"

// Forward declarations
class UEditableTextBox;
class UComboBoxString;
class UButton;
class UTextBlock;

/**
 * UCreateSessionWidget
 *
 * Đây là widget hiển thị form để người chơi nhập:
 * - Room Name
 * - Password
 * - Max Players
 * - Map Selection
 *
 * Khi bấm nút Create, widget phát đi một delegate chứa FSessionSettingsData.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateSessionRequest, const FSessionSettingsData&, SessionData);

UCLASS()
class HORRORGAME_API UCreateSessionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UCreateSessionWidget(const FObjectInitializer& ObjInit);

    /** Delegate mà parent (ví dụ UGameModeSelection) bind để lắng nghe khi user nhấn Create */
    UPROPERTY(BlueprintAssignable, Category = "Session")
    FOnCreateSessionRequest OnCreateSessionRequest;

protected:
    virtual void NativeConstruct() override;

    /** Input text cho Room Name */
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* TextBox_RoomName;

    /** Input text cho Password */
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* TextBox_Password;

    /** ComboBox để chọn Max Players (ví dụ 2, 4, 8) */
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* ComboBox_MaxPlayers;

    /** ComboBox để chọn Map */
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* ComboBox_MapSelection;

    /** Nút "Create" */
    UPROPERTY(meta = (BindWidget))
    UButton* Button_Create;

    /** Nút "Cancel" */
    UPROPERTY(meta = (BindWidget))
    UButton* Button_Cancel;

    UPROPERTY(meta = (BindWidget))
	UButton* Button_Back;

    /** TextBlock để hiển help/error (tùy chọn) */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ErrorMessage;

    // =======================
    // ===   EVENT HANDLES ===
    // =======================

    /** Khi bấm nút Create */
    UFUNCTION()
    void HandleOnCreateClicked();

    /** Khi bấm nút Cancel */
    UFUNCTION()
    void HandleOnCancelClicked();

    UFUNCTION()
    void OnBackCreateSession();

    /** Kiểm tra dữ liệu đầu vào, trả về true nếu hợp lệ */
    bool ValidateInput(FSessionSettingsData& OutData);

    /** Helper: Chuyển chuỗi ComboBox sang int */
    int32 ConvertComboBoxToInt(const FString& InString) const;
};
