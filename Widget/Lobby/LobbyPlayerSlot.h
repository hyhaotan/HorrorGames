#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/WidgetAnimation.h"
#include "LobbyPlayerSlot.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotInteraction, int32, SlotIndex);

UCLASS()
class HORRORGAME_API ULobbyPlayerSlot : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // Slot management
    UFUNCTION(BlueprintCallable)
    void SetPlayerInfo(const FString& PlayerName, bool bInIsHost);

    UFUNCTION(BlueprintCallable)
    void SetPlayerInfoWithAnimation(const FString& PlayerName, bool bInIsHost);

    UFUNCTION(BlueprintCallable)
    void SetEmpty();

    UFUNCTION(BlueprintCallable)
    void SetSlotIndex(int32 Index) { SlotIndex = Index; }

    // State getters
    UFUNCTION(BlueprintCallable)
    bool IsOccupied() const { return bIsOccupied; }

    UFUNCTION(BlueprintCallable)
    bool IsHost() const { return bIsHost; }

    UFUNCTION(BlueprintCallable)
    FString GetPlayerName() const { return CurrentPlayerName; }

    // Material setup
    UFUNCTION(BlueprintCallable)
    void SetDynamicMaterials(UMaterialInstanceDynamic* HostMat, UMaterialInstanceDynamic* NormalMat, UMaterialInstanceDynamic* EmptyMat);

    // Animation functions
    UFUNCTION(BlueprintImplementableEvent)
    void PlaySlotJoinAnimation();

    UFUNCTION(BlueprintImplementableEvent)
    void PlaySlotLeaveAnimation();

    UFUNCTION(BlueprintImplementableEvent)
    void PlaySlotAppearAnimation();

    UFUNCTION(BlueprintImplementableEvent)
    void PlayHostGlowAnimation();

    UFUNCTION(BlueprintImplementableEvent)
    void PlayInviteClickAnimation();

    // Interaction delegates
    UPROPERTY(BlueprintAssignable)
    FOnSlotInteraction OnInviteClicked;

    UPROPERTY(BlueprintAssignable)
    FOnSlotInteraction OnKickClicked;

protected:
    // UI Components
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget))
    UButton* InviteButton;

    UPROPERTY(meta = (BindWidget))
    UButton* KickButton;

    UPROPERTY(meta = (BindWidget))
    UImage* PlayerAvatar;

    UPROPERTY(meta = (BindWidget))
    UImage* HostIcon;

    UPROPERTY(meta = (BindWidget))
    UBorder* SlotBorder;

    // Animations
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* JoinAnim;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* LeaveAnim;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* HostGlowAnim;

private:
    // Slot state
    bool bIsOccupied = false;
    bool bIsHost = false;
    FString CurrentPlayerName;
    int32 SlotIndex = 0;

    // Dynamic materials
    UPROPERTY()
    UMaterialInstanceDynamic* HostBorderMaterialDynamic;

    UPROPERTY()
    UMaterialInstanceDynamic* NormalBorderMaterialDynamic;

    UPROPERTY()
    UMaterialInstanceDynamic* EmptyBorderMaterialDynamic;

    UPROPERTY()
    FTimerHandle CrownTimer;

    // Button event handlers
    UFUNCTION()
    void OnInviteButtonClicked();

    UFUNCTION()
    void OnKickButtonClicked();

    // Update functions
    void UpdateSlotContent();
    void UpdateButtonVisibility();
    void ApplyMaterialStyling();
    void SetHostStyling(bool bIsHost);
};