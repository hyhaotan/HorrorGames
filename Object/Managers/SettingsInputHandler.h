#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "SettingsInputHandler.generated.h"

class UMenuSettingSystemWidget;
class USettingsTabManager;
class USettingsStateManager;

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API USettingsInputHandler : public UObject
{
    GENERATED_BODY()

public:
    USettingsInputHandler();

    // === INITIALIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Input Handler")
    void Initialize(UMenuSettingSystemWidget* InParentWidget);

    // === INPUT HANDLING ===

    FReply HandleKeyDown(const FKeyEvent& InKeyEvent);

    UFUNCTION(BlueprintCallable, Category = "Input Handler")
    bool HandleControllerInput(const FKey& Key);

    // === KEYBOARD SHORTCUTS ===

    UFUNCTION(BlueprintCallable, Category = "Input Handler")
    void SetupKeyboardShortcuts();

    UFUNCTION(BlueprintCallable, Category = "Input Handler")
    void EnableKeyboardShortcuts(bool bEnabled);

    // === EXTERNAL MANAGER REFERENCES ===

    UFUNCTION(BlueprintCallable, Category = "Input Handler")
    void SetTabManager(USettingsTabManager* InTabManager);

    UFUNCTION(BlueprintCallable, Category = "Input Handler")
    void SetStateManager(USettingsStateManager* InStateManager);

protected:
    // === INPUT PROCESSING ===

    bool HandleTabNavigation(const FKeyEvent& InKeyEvent);
    bool HandleActionShortcuts(const FKeyEvent& InKeyEvent);
    bool HandleNumberShortcuts(const FKeyEvent& InKeyEvent);
    bool HandleSpecialKeys(const FKeyEvent& InKeyEvent);

private:
    // === REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UMenuSettingSystemWidget> ParentWidget;

    UPROPERTY()
    TWeakObjectPtr<USettingsTabManager> TabManager;

    UPROPERTY()
    TWeakObjectPtr<USettingsStateManager> StateManager;

    // === INPUT STATE ===

    UPROPERTY()
    bool bKeyboardShortcutsEnabled;

    // === SHORTCUT MAPPINGS ===

    UPROPERTY()
    TMap<FKey, FString> KeyboardShortcuts;

    // === HELPER FUNCTIONS ===

    void InitializeDefaultShortcuts();
    bool ExecuteShortcut(const FString& ShortcutName);
    bool IsModifierKeyDown(const FKeyEvent& InKeyEvent, const FString& ModifierName) const;

public:
    // === DEBUG FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugPrintShortcuts() const;
};