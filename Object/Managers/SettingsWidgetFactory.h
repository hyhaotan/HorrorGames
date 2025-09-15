#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/WidgetSwitcher.h"
#include "SettingsWidgetFactory.generated.h"

class UMenuSettingSystemWidget;
class UGameplayWidget;
class UGraphicsWidget;
class UAudioWidget;
class UVoiceWidget;
class UControlsWidget;
class UAccessibilityWidget;

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API USettingsWidgetFactory : public UObject
{
    GENERATED_BODY()

public:
    USettingsWidgetFactory();

    // === INITIALIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Widget Factory")
    void Initialize(UMenuSettingSystemWidget* InParentWidget);

    UFUNCTION(BlueprintCallable, Category = "Widget Factory")
    void CreateAllChildWidgets();

    // === WIDGET REFRESH ===

    UFUNCTION(BlueprintCallable, Category = "Widget Factory")
    void RefreshAllWidgets();

    UFUNCTION(BlueprintCallable, Category = "Widget Factory")
    void RefreshCurrentWidget(int32 CurrentTabIndex);

    UFUNCTION(BlueprintCallable, Category = "Widget Factory")
    void RefreshWidgetByIndex(int32 WidgetIndex);

    // === WIDGET GETTERS ===

    UFUNCTION(BlueprintPure, Category = "Widget Factory")
    UGameplayWidget* GetGameplayWidget() const { return GameplayWidget; }

    UFUNCTION(BlueprintPure, Category = "Widget Factory")
    UGraphicsWidget* GetGraphicsWidget() const { return GraphicsWidget; }

    UFUNCTION(BlueprintPure, Category = "Widget Factory")
    UAudioWidget* GetAudioWidget() const { return AudioWidget; }

    UFUNCTION(BlueprintPure, Category = "Widget Factory")
    UVoiceWidget* GetVoiceWidget() const { return VoiceWidget; }

    UFUNCTION(BlueprintPure, Category = "Widget Factory")
    UControlsWidget* GetControlsWidget() const { return ControlsWidget; }

    UFUNCTION(BlueprintPure, Category = "Widget Factory")
    UAccessibilityWidget* GetAccessibilityWidget() const { return AccessibilityWidget; }

    UFUNCTION(BlueprintPure, Category = "Widget Factory")
    UWidget* GetWidgetByIndex(int32 Index) const;

    // === WIDGET VALIDATION ===

    UFUNCTION(BlueprintCallable, Category = "Widget Factory")
    bool ValidateAllWidgets() const;

    UFUNCTION(BlueprintCallable, Category = "Widget Factory")
    bool IsWidgetValid(int32 WidgetIndex) const;

protected:
    // === WIDGET CREATION ===

    bool CreateGameplayWidget();
    bool CreateGraphicsWidget();
    bool CreateAudioWidget();
    bool CreateVoiceWidget();
    bool CreateControlsWidget();
    bool CreateAccessibilityWidget();

    // === WIDGET REFRESH HELPERS ===

    void RefreshSpecificWidget(UWidget* Widget);
    bool CallWidgetRefreshFunction(UWidget* Widget, const FString& FunctionName);
    void ValidateWidgetCreation();
    UUserWidget* CreateFallbackWidget(const FString& LabelText);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UGameplayWidget> GameplayWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UGraphicsWidget> GraphicsWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UAudioWidget> AudioWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UVoiceWidget> VoiceWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UControlsWidget> ControlsWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UAccessibilityWidget> AccessibilityWidgetClass;
private:
    // === CHILD WIDGET REFERENCES ===

    UPROPERTY()
    UGameplayWidget* GameplayWidget;

    UPROPERTY()
    UGraphicsWidget* GraphicsWidget;

    UPROPERTY()
    UAudioWidget* AudioWidget;

    UPROPERTY()
    UVoiceWidget* VoiceWidget;

    UPROPERTY()
    UControlsWidget* ControlsWidget;

    UPROPERTY()
    UAccessibilityWidget* AccessibilityWidget;

    // === UI REFERENCES ===

    UPROPERTY()
    TWeakObjectPtr<UMenuSettingSystemWidget> ParentWidget;

    UPROPERTY()
    TWeakObjectPtr<UWidgetSwitcher> ContentSwitcher;

    // === WIDGET ARRAY FOR EASY ACCESS ===

    UPROPERTY()
    TArray<UWidget*> AllChildWidgets;

    // === HELPER FUNCTIONS ===

    void AddWidgetToSwitcher(UWidget* Widget);
    void CacheAllWidgets();
    bool IsWidgetIndexValid(int32 Index) const;
    bool CreateSimpleFallbackWidget(const FString& LabelText);

public:
    // === DEBUG FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugPrintWidgetInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugValidateAllWidgets() const;
};