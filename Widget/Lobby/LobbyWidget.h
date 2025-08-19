#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Animation/WidgetAnimation.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"
#include "LobbyWidget.generated.h"

class ULobbyPlayerSlot;

// Delegate declarations for slot interactions
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotInviteClicked, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotKickClicked, int32, SlotIndex);

UCLASS()
class HORRORGAME_API ULobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

    // Player management
    UFUNCTION(BlueprintCallable)
    void UpdatePlayerSlots(const TArray<FString>& PlayerNames, const TArray<bool>& IsHostArray);

    UFUNCTION(BlueprintCallable)
    void OnPlayerJoined(const FString& PlayerName, bool bIsHost);

    UFUNCTION(BlueprintCallable)
    void OnPlayerLeft(const FString& PlayerName);

    // Slot interaction delegates
    UPROPERTY(BlueprintAssignable)
    FOnSlotInviteClicked OnSlotInviteClicked;

    UPROPERTY(BlueprintAssignable)
    FOnSlotKickClicked OnSlotKickClicked;

protected:
    // Core UI Components
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* PlayerSlotsContainer;

    UPROPERTY(meta = (BindWidget))
    UButton* StartGameButton;

    UPROPERTY(meta = (BindWidget))
    UButton* InviteFriendsButton;

    UPROPERTY(meta = (BindWidget))
    UButton* LeaveButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LobbyStatusText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LobbyTitleText;

    UPROPERTY(meta = (BindWidget))
    class UWidget* CharacterPreviewArea;

    // Animations
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowLobbyAnim;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* HideLobbyAnim;

    // Styling Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling")
    FLinearColor AccentColor = FLinearColor(1.0f, 0.267f, 0.267f, 1.0f); // DBD Red

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling")
    FLinearColor HostColor = FLinearColor(1.0f, 0.843f, 0.0f, 1.0f); // Gold

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* HostBorderMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* NormalBorderMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* EmptyBorderMaterial;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* LobbyEnterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* PlayerJoinSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* PlayerLeaveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ButtonHoverSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ButtonPressSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* StartGameSound;

    // Player Slot Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    TSubclassOf<ULobbyPlayerSlot> PlayerSlotClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    int32 MaxPlayers = 4;

private:
    // Player slots array
    UPROPERTY()
    TArray<ULobbyPlayerSlot*> PlayerSlots;

    UPROPERTY()
    TMap<UTextBlock*, FTimerHandle> TextAnimationTimers;

    // Dynamic materials
    UPROPERTY()
    UMaterialInstanceDynamic* HostBorderMaterialDynamic;

    UPROPERTY()
    UMaterialInstanceDynamic* NormalBorderMaterialDynamic;

    UPROPERTY()
    UMaterialInstanceDynamic* EmptyBorderMaterialDynamic;

    // Session interface
    IOnlineSessionPtr SessionInterface;

    // Initialization functions
    void CreateDynamicMaterials();
    void ApplyInitialStyling();
    void BindButtonEvents();
    void InitializePlayerSlots();
    void PlayEntranceAnimation();
    void InitializeSessionInterface();

    // Button event handlers
    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnInviteFriendsClicked();

    UFUNCTION()
    void OnLeaveClicked();

    UFUNCTION()
    void OnButtonHovered();

    UFUNCTION()
    void OnButtonPressed();

    // Slot event handlers
    UFUNCTION()
    void OnSlotInviteClickedInternal(int32 SlotIndex);

    UFUNCTION()
    void OnSlotKickClickedInternal(int32 SlotIndex);

    // UI update functions
    void UpdateLobbyStatusWithAnimation();
    void UpdateStartButtonState(int32 PlayerCount);
    bool IsLocalPlayerHost() const;

    // Animation utilities
    void AnimateTextColor(UTextBlock* TextWidget, const FLinearColor& TargetColor, float Duration);
    void AnimateButtonGlow(UButton* Button, bool bShouldGlow);
};