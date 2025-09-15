#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "HorrorGame/Data/SettingsStructs.h"
#include "SettingsTabManager.generated.h"

class UMenuSettingSystemWidget;
class USettingsAudioManager;

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API USettingsTabManager : public UObject
{
    GENERATED_BODY()

public:
    USettingsTabManager();
    // === INITIALIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void Initialize(UMenuSettingSystemWidget* InParentWidget);

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void BindTabButtons();

    UFUNCTION(BlueprintCallable, Category = "Settings Tab Manager")
    void SetInitialTab();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintTabInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugWidgetVisibility(int32 TabIndex);

    // === TAB SWITCHING ===

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void SwitchToTab(int32 TabIndex, bool bPlaySound = true);

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void NextTab();

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void PreviousTab();

    UFUNCTION(BlueprintPure, Category = "Tab Management")
    UButton* GetTabButton(int32 Index) const;

    // === TAB STATE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void UpdateAllTabStates();

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void SetTabButtonStyle(UButton* Button, bool bSelected, int32 TabIndex);

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void AnimateTabTransition(int32 FromTab, int32 ToTab);

    // === GETTERS ===

    UFUNCTION(BlueprintPure, Category = "Tab Management")
    int32 GetCurrentTab() const { return CurrentTab; }

    UFUNCTION(BlueprintPure, Category = "Tab Management")
    int32 GetTabCount() const { return TabData.Num(); }

    UFUNCTION(BlueprintPure, Category = "Tab Management")
    const TArray<FTabInfo>& GetTabData() const { return TabData; }

    UFUNCTION(BlueprintPure, Category = "Tab Management")
    bool IsValidTabIndex(int32 TabIndex) const;

    // === EXTERNAL REFERENCES ===

    UFUNCTION(BlueprintCallable, Category = "Tab Management")
    void SetAudioManager(USettingsAudioManager* InAudioManager);

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "Tab Events")
    FOnTabChangedDelegate OnTabChanged;

protected:
    // === EVENT HANDLERS ===

    UFUNCTION()
    void OnGameplayTabClicked();

    UFUNCTION()
    void OnGraphicsTabClicked();

    UFUNCTION()
    void OnAudioTabClicked();

    UFUNCTION()
    void OnVoiceTabClicked();

    UFUNCTION()
    void OnControlsTabClicked();

    UFUNCTION()
    void OnAccessibilityTabClicked();

    UFUNCTION()
    void OnTabButtonHovered();
    UFUNCTION()
    void OnTabButtonUnhovered();

private:
    // === CORE DATA ===

    UPROPERTY()
    int32 CurrentTab;

    UPROPERTY()
    TArray<FTabInfo> TabData;

    UPROPERTY()
    TArray<UButton*> TabButtons;

    // === REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UMenuSettingSystemWidget> ParentWidget;

    UPROPERTY()
    TWeakObjectPtr<USettingsAudioManager> AudioManager;

    UPROPERTY()
    TWeakObjectPtr<UWidgetSwitcher> ContentSwitcher;

    UPROPERTY()
    TWeakObjectPtr<UTextBlock> TabDescriptionText;

    // === STYLING ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling", meta = (AllowPrivateAccess = "true"))
    FLinearColor SelectedTabColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling", meta = (AllowPrivateAccess = "true"))
    FLinearColor UnselectedTabColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling", meta = (AllowPrivateAccess = "true"))
    FVector2D SelectedTabScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling", meta = (AllowPrivateAccess = "true"))
    FVector2D NormalTabScale;

    // === HELPER FUNCTIONS ===

    void InitializeTabData();
    void UpdateTabDescription(int32 TabIndex);
    int32 FindNextValidTab(int32 StartIndex, bool bForward) const;
};