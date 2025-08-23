#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SettingsManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsChanged, FName, SettingName);

UCLASS()
class HORRORGAME_API USettingsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyGraphicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyAudioSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyGameplaySettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyControlSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyAccessibilitySettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SaveAllSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void LoadAllSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    bool HasUnsavedChanges() const { return bHasUnsavedChanges; }

    UPROPERTY(BlueprintAssignable)
    FOnSettingsChanged OnSettingsChanged;

private:
    void OnLanguageChanged(const FString& NewLanguage);
    void OnUIScaleChanged(float NewScale);

    bool bHasUnsavedChanges = false;
};