#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Data/SessionSettingsData.h"
#include "CreateSessionWidget.generated.h"

class UEditableTextBox;
class UComboBoxString;
class UButton;
class UTextBlock;

UCLASS()
class HORRORGAME_API UCreateSessionWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget)) UEditableTextBox* TextBox_RoomName;
    UPROPERTY(meta = (BindWidget)) UEditableTextBox* TextBox_Password;
    UPROPERTY(meta = (BindWidget)) UComboBoxString* ComboBox_MaxPlayers;
    UPROPERTY(meta = (BindWidget)) UComboBoxString* ComboBox_MapSelection;
    UPROPERTY(meta = (BindWidget)) UButton* Button_Create;
    UPROPERTY(meta = (BindWidget)) UButton* Button_Cancel;
    UPROPERTY(meta = (BindWidget)) UTextBlock* Text_ErrorMessage;

    FDelegateHandle CreateSessionCompleteHandle;

    bool ValidateInput(FSessionSettingsData& OutData);
    int32 ConvertStringToInt(const FString& In) const;

    UFUNCTION()
    void HandleCreateClicked();

    UFUNCTION()
    void HandleCancelClicked();

    void CreateHostSession(const FSessionSettingsData& Data);

    UFUNCTION()
    void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
};