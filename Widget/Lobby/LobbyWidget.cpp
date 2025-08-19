#include "LobbyWidget.h"
#include "LobbyPlayerSlot.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSessionSettings.h"

void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Create dynamic materials first
    CreateDynamicMaterials();

    // Setup initial styling
    ApplyInitialStyling();

    // Bind all button events
    BindButtonEvents();

    // Initialize player slots with staggered animations
    InitializePlayerSlots();

    // Play entrance animation
    PlayEntranceAnimation();

    // Initialize session interface
    InitializeSessionInterface();

    // Initial status update
    UpdateLobbyStatusWithAnimation();
}

void ULobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update any continuous animations or effects here
    // For example, host glow intensity based on time
    static float GlowTime = 0.0f;
    GlowTime += InDeltaTime;

    if (HostBorderMaterialDynamic)
    {
        float GlowIntensity = 0.5f + 0.3f * FMath::Sin(GlowTime * 2.0f);
        HostBorderMaterialDynamic->SetScalarParameterValue(TEXT("GlowIntensity"), GlowIntensity);
    }
}

void ULobbyWidget::NativeDestruct()
{
    // Clear all text animation timers
    for (auto& Pair : TextAnimationTimers)
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(Pair.Value);
        }
    }
    TextAnimationTimers.Empty();

    Super::NativeDestruct();
}

void ULobbyWidget::CreateDynamicMaterials()
{
    if (HostBorderMaterial)
    {
        HostBorderMaterialDynamic = UMaterialInstanceDynamic::Create(HostBorderMaterial, this);
        if (HostBorderMaterialDynamic)
        {
            HostBorderMaterialDynamic->SetVectorParameterValue(TEXT("BorderColor"), HostColor);
            HostBorderMaterialDynamic->SetScalarParameterValue(TEXT("GlowIntensity"), 0.8f);
        }
    }

    if (NormalBorderMaterial)
    {
        NormalBorderMaterialDynamic = UMaterialInstanceDynamic::Create(NormalBorderMaterial, this);
        if (NormalBorderMaterialDynamic)
        {
            NormalBorderMaterialDynamic->SetVectorParameterValue(TEXT("BorderColor"), FLinearColor::White);
            NormalBorderMaterialDynamic->SetScalarParameterValue(TEXT("GlowIntensity"), 0.3f);
        }
    }

    if (EmptyBorderMaterial)
    {
        EmptyBorderMaterialDynamic = UMaterialInstanceDynamic::Create(EmptyBorderMaterial, this);
        if (EmptyBorderMaterialDynamic)
        {
            EmptyBorderMaterialDynamic->SetVectorParameterValue(TEXT("BorderColor"), FLinearColor::Gray);
            EmptyBorderMaterialDynamic->SetScalarParameterValue(TEXT("PulseSpeed"), 0.5f);
        }
    }
}

void ULobbyWidget::ApplyInitialStyling()
{
    // Style the main title
    if (LobbyTitleText)
    {
        LobbyTitleText->SetColorAndOpacity(AccentColor);

        // Apply shadow/glow effect
        FSlateFontInfo FontInfo = LobbyTitleText->GetFont();
        FontInfo.Size = 32;
        FontInfo.OutlineSettings.OutlineSize = 2;
        FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
        LobbyTitleText->SetFont(FontInfo);
    }

    // Style the status text
    if (LobbyStatusText)
    {
        LobbyStatusText->SetColorAndOpacity(HostColor);

        FSlateFontInfo StatusFont = LobbyStatusText->GetFont();
        StatusFont.Size = 18;
        LobbyStatusText->SetFont(StatusFont);
    }
}

void ULobbyWidget::BindButtonEvents()
{
    if (StartGameButton)
    {
        StartGameButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartGameClicked);
        StartGameButton->OnHovered.AddDynamic(this, &ULobbyWidget::OnButtonHovered);
        StartGameButton->OnPressed.AddDynamic(this, &ULobbyWidget::OnButtonPressed);
    }

    if (InviteFriendsButton)
    {
        InviteFriendsButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnInviteFriendsClicked);
        InviteFriendsButton->OnHovered.AddDynamic(this, &ULobbyWidget::OnButtonHovered);
    }

    if (LeaveButton)
    {
        LeaveButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnLeaveClicked);
        LeaveButton->OnHovered.AddDynamic(this, &ULobbyWidget::OnButtonHovered);
    }
}

void ULobbyWidget::InitializePlayerSlots()
{
    if (!PlayerSlotsContainer || !PlayerSlotClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerSlotsContainer or PlayerSlotClass is null!"));
        return;
    }

    // Clear existing slots
    PlayerSlots.Empty();
    PlayerSlotsContainer->ClearChildren();

    // Create new slots with staggered animations
    for (int32 i = 0; i < MaxPlayers; i++)
    {
        ULobbyPlayerSlot* NewSlot = CreateWidget<ULobbyPlayerSlot>(this, PlayerSlotClass);
        if (NewSlot)
        {
            // Setup slot properties
            NewSlot->SetSlotIndex(i);
            NewSlot->SetDynamicMaterials(
                HostBorderMaterialDynamic,
                NormalBorderMaterialDynamic,
                EmptyBorderMaterialDynamic
            );

            // Add to container and array
            PlayerSlotsContainer->AddChild(NewSlot);
            PlayerSlots.Add(NewSlot);

            // Bind slot interaction events
            NewSlot->OnInviteClicked.AddDynamic(this, &ULobbyWidget::OnSlotInviteClickedInternal);
            NewSlot->OnKickClicked.AddDynamic(this, &ULobbyWidget::OnSlotKickClickedInternal);

            // Animate slot appearance with delay
            FTimerHandle SlotAnimTimer;
            float AnimDelay = 0.15f * i;

            TWeakObjectPtr<ULobbyPlayerSlot> WeakSlot = NewSlot;
            GetWorld()->GetTimerManager().SetTimer(SlotAnimTimer, [WeakSlot]() {
                if (WeakSlot.IsValid()) {
                    WeakSlot->PlaySlotAppearAnimation();
                    WeakSlot->SetEmpty();
                }
            }, AnimDelay, false);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Created %d player slots"), PlayerSlots.Num());
}

void ULobbyWidget::PlayEntranceAnimation()
{
    if (ShowLobbyAnim)
    {
        PlayAnimation(ShowLobbyAnim);
    }

    // Play entrance sound
    if (LobbyEnterSound)
    {
        UGameplayStatics::PlaySound2D(this, LobbyEnterSound, 0.7f);
    }
}

void ULobbyWidget::InitializeSessionInterface()
{
    // Get any available online subsystem
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        UE_LOG(LogTemp, Log, TEXT("Lobby using Online Subsystem: %s"),
            *OnlineSubsystem->GetSubsystemName().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Online Subsystem available - running in offline mode"));
    }
}

void ULobbyWidget::UpdatePlayerSlots(const TArray<FString>& PlayerNames, const TArray<bool>& IsHostArray)
{
    if (PlayerNames.Num() != IsHostArray.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerNames and IsHostArray size mismatch"));
        return;
    }

    // Update each slot with animation
    for (int32 i = 0; i < PlayerSlots.Num(); i++)
    {
        if (i < PlayerNames.Num() && PlayerSlots[i])
        {
            if (!PlayerNames[i].IsEmpty())
            {
                PlayerSlots[i]->SetPlayerInfoWithAnimation(PlayerNames[i], IsHostArray[i]);
            }
            else
            {
                PlayerSlots[i]->SetEmpty();
            }
        }
        else if (PlayerSlots[i])
        {
            PlayerSlots[i]->SetEmpty();
        }
    }

    UpdateLobbyStatusWithAnimation();
}

void ULobbyWidget::OnPlayerJoined(const FString& PlayerName, bool bIsHost)
{
    UE_LOG(LogTemp, Log, TEXT("Player joined: %s (Host: %s)"), *PlayerName, bIsHost ? TEXT("Yes") : TEXT("No"));

    // Find first empty slot
    for (ULobbyPlayerSlot* Slots : PlayerSlots)
    {
        if (Slots && !Slots->IsOccupied())
        {
            // Set player info with join animation
            Slots->SetPlayerInfoWithAnimation(PlayerName, bIsHost);

            // Play join sound
            if (PlayerJoinSound)
            {
                UGameplayStatics::PlaySound2D(this, PlayerJoinSound, 0.8f);
            }

            break;
        }
    }

    UpdateLobbyStatusWithAnimation();
}

void ULobbyWidget::OnPlayerLeft(const FString& PlayerName)
{
    UE_LOG(LogTemp, Log, TEXT("Player left: %s"), *PlayerName);

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerLeft: GetWorld() == nullptr"));
        return;
    }

    for (ULobbyPlayerSlot* Slotss : PlayerSlots)
    {
        if (Slotss && Slotss->IsOccupied() && Slotss->GetPlayerName() == PlayerName)
        {
            // Play leave animation
            Slotss->PlaySlotLeaveAnimation();

            // Use weak pointer to avoid dangling access
            TWeakObjectPtr<ULobbyPlayerSlot> WeakSlot = Slotss;

            // Prepare delegate that checks validity before calling
            FTimerDelegate Delegate = FTimerDelegate::CreateLambda([WeakSlot]()
                {
                    if (WeakSlot.IsValid())
                    {
                        WeakSlot->SetEmpty();
                    }
                    // else: slot was destroyed -> do nothing
                });

            FTimerHandle LeaveTimer;
            World->GetTimerManager().SetTimer(LeaveTimer, Delegate, 0.5f, false);

            if (PlayerLeaveSound)
            {
                UGameplayStatics::PlaySound2D(this, PlayerLeaveSound, 0.6f);
            }
            break;
        }
    }

    UpdateLobbyStatusWithAnimation();
}


void ULobbyWidget::UpdateLobbyStatusWithAnimation()
{
    if (!LobbyStatusText) return;

    // Count current players
    int32 CurrentPlayers = 0;
    for (const ULobbyPlayerSlot* Slotes : PlayerSlots)
    {
        if (Slotes && Slotes->IsOccupied())
        {
            CurrentPlayers++;
        }
    }

    // Create status text with color coding
    FString StatusString = FString::Printf(TEXT("SURVIVORS: %d/%d"), CurrentPlayers, MaxPlayers);
    LobbyStatusText->SetText(FText::FromString(StatusString));

    // Animate color based on player count
    FLinearColor StatusColor = HostColor;
    if (CurrentPlayers == 0)
    {
        StatusColor = FLinearColor::Red;
    }
    else if (CurrentPlayers == MaxPlayers)
    {
        StatusColor = FLinearColor::Green;
    }

    AnimateTextColor(LobbyStatusText, StatusColor, 0.3f);

    // Update start button
    UpdateStartButtonState(CurrentPlayers);
}

void ULobbyWidget::UpdateStartButtonState(int32 PlayerCount)
{
    if (!StartGameButton) return;

    bool bCanStart = IsLocalPlayerHost() && PlayerCount >= 1;
    StartGameButton->SetIsEnabled(bCanStart);

    // Update button text and styling
    FString ButtonText = TEXT("READY");
    if (PlayerCount >= MaxPlayers)
    {
        ButtonText = TEXT("START MATCH");
    }
    else if (PlayerCount == 0)
    {
        ButtonText = TEXT("WAITING...");
    }

    // Get button text widget and update
    if (UTextBlock* ButtonTextWidget = Cast<UTextBlock>(StartGameButton->GetChildAt(0)))
    {
        ButtonTextWidget->SetText(FText::FromString(ButtonText));
    }

    // Visual feedback for state
    AnimateButtonGlow(StartGameButton, bCanStart);
}

bool ULobbyWidget::IsLocalPlayerHost() const
{
    if (!SessionInterface.IsValid())
    {
        // In offline mode, treat as host if we have players
        return PlayerSlots.Num() > 0;
    }

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    if (Session)
    {
        APlayerController* PC = GetOwningPlayer();
        if (PC && PC->PlayerState)
        {
            return Session->OwningUserId == PC->PlayerState->GetUniqueId();
        }
    }

    return false;
}

void ULobbyWidget::AnimateTextColor(UTextBlock* TextWidget, const FLinearColor& TargetColor, float Duration)
{
    if (!TextWidget || Duration <= 0.0f) return;

    // Clear any existing animation for this widget
    if (TextAnimationTimers.Contains(TextWidget))
    {
        GetWorld()->GetTimerManager().ClearTimer(TextAnimationTimers[TextWidget]);
        TextAnimationTimers.Remove(TextWidget);
    }

    FLinearColor StartColor = TextWidget->GetColorAndOpacity().GetSpecifiedColor();
    auto ElapsedTime = MakeShared<float>(0.0f);

    TWeakObjectPtr<UTextBlock> WeakTextWidget = TextWidget;
    FTimerHandle NewTimer;
    GetWorld()->GetTimerManager().SetTimer(NewTimer,
        [this, WeakTextWidget, StartColor, TargetColor, Duration, ElapsedTime]()
        {
            if (!WeakTextWidget.IsValid() || !TextAnimationTimers.Contains(WeakTextWidget.Get()))
            {
                return;
            }

            *ElapsedTime += 0.016f;
            float Alpha = FMath::Clamp(*ElapsedTime / Duration, 0.0f, 1.0f);
            FLinearColor CurrentColor = FMath::Lerp(StartColor, TargetColor, FMath::SmoothStep(0.0f, 1.0f, Alpha));

            WeakTextWidget->SetColorAndOpacity(CurrentColor);

            if (Alpha >= 1.0f)
            {
                GetWorld()->GetTimerManager().ClearTimer(TextAnimationTimers[WeakTextWidget.Get()]);
                TextAnimationTimers.Remove(WeakTextWidget.Get());
            }
        }, 0.016f, true);


    // Store timer handle
    TextAnimationTimers.Add(TextWidget, NewTimer);
}

void ULobbyWidget::AnimateButtonGlow(UButton* Button, bool bShouldGlow)
{
    if (!Button) return;

    // Get current button style
    FButtonStyle CurrentStyle = Button->GetStyle();

    // Try to get existing dynamic material or create new one
    UMaterialInstanceDynamic* ButtonMaterial = nullptr;

    // Check if we already have a dynamic material
    if (UMaterialInstanceDynamic* ExistingDynamic = Cast<UMaterialInstanceDynamic>(CurrentStyle.Normal.GetResourceObject()))
    {
        ButtonMaterial = ExistingDynamic;
    }
    else if (UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(CurrentStyle.Normal.GetResourceObject()))
    {
        // Create dynamic material from base material
        ButtonMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Button);

        if (ButtonMaterial)
        {
            // Update button style with new dynamic material
            FButtonStyle NewStyle = CurrentStyle;
            NewStyle.Normal.SetResourceObject(ButtonMaterial);
            NewStyle.Hovered.SetResourceObject(ButtonMaterial);
            NewStyle.Pressed.SetResourceObject(ButtonMaterial);
            Button->SetStyle(NewStyle);
        }
    }

    // Apply glow parameters if we have a dynamic material
    if (ButtonMaterial)
    {
        float TargetGlow = bShouldGlow ? 1.0f : 0.3f;
        FLinearColor GlowColor = bShouldGlow ? AccentColor : FLinearColor::Gray;

        // Set material parameters (make sure these parameter names exist in your material)
        ButtonMaterial->SetScalarParameterValue(TEXT("GlowIntensity"), TargetGlow);
        ButtonMaterial->SetVectorParameterValue(TEXT("GlowColor"), GlowColor);

        // Optional: Add opacity/alpha parameter
        ButtonMaterial->SetScalarParameterValue(TEXT("Opacity"), bShouldGlow ? 1.0f : 0.7f);
    }
    else
    {
        // Fallback: Use simple color tinting if no material is available
        FButtonStyle NewStyle = CurrentStyle;
        FLinearColor TintColor = bShouldGlow ? AccentColor : FLinearColor::Gray;

        NewStyle.Normal.TintColor = FSlateColor(TintColor);
        NewStyle.Hovered.TintColor = FSlateColor(TintColor * 1.2f); // Slightly brighter on hover
        NewStyle.Pressed.TintColor = FSlateColor(TintColor * 0.8f); // Slightly darker when pressed

        Button->SetStyle(NewStyle);
    }
}

// Button Event Handlers
void ULobbyWidget::OnStartGameClicked()
{
    int32 PlayerCount = 0;
    for (const ULobbyPlayerSlot* Slot1 : PlayerSlots)
    {
        if (Slot1 && Slot1->IsOccupied()) PlayerCount++;
    }

    if (!IsLocalPlayerHost())
    {
        UE_LOG(LogTemp, Warning, TEXT("Only host can start the game"));
        return;
    }

    if (PlayerCount < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Need at least 1 player to start"));
        return;
    }

    // Play start sound
    if (StartGameSound)
    {
        UGameplayStatics::PlaySound2D(this, StartGameSound);
    }

    // Start the game
    UE_LOG(LogTemp, Log, TEXT("Starting game with %d players"), PlayerCount);

    // Play exit animation then load game
    if (HideLobbyAnim)
    {
        PlayAnimation(HideLobbyAnim);

        FTimerHandle StartTimer;
        GetWorld()->GetTimerManager().SetTimer(StartTimer, []()
            {
                UGameplayStatics::OpenLevel(GWorld, TEXT("L_horrorGame"), true, TEXT("listen"));
            }, 1.0f, false);
    }
    else
    {
        UGameplayStatics::OpenLevel(this, TEXT("L_horrorGame"), true, TEXT("listen"));
    }
}

void ULobbyWidget::OnInviteFriendsClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Invite friends clicked"));

    // Try Steam integration first
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

    if (OnlineSubsystem && OnlineSubsystem->GetSubsystemName() == TEXT("Steam"))
    {
        IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
        if (ExternalUI.IsValid())
        {
            ExternalUI->ShowFriendsUI(0);
            return;
        }
    }

    // Fallback: show generic invite message
    UE_LOG(LogTemp, Log, TEXT("Steam overlay not available - showing generic invite"));
}

void ULobbyWidget::OnLeaveClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Leave lobby clicked"));

    // Clean up session if host
    if (SessionInterface.IsValid() && IsLocalPlayerHost())
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }

    // Return to main menu
    if (HideLobbyAnim)
    {
        PlayAnimation(HideLobbyAnim);

        FTimerHandle LeaveTimer;
        GetWorld()->GetTimerManager().SetTimer(LeaveTimer, []()
            {
                UGameplayStatics::OpenLevel(GWorld, TEXT("MainMenu"), true);
            }, 1.0f, false);
    }
    else
    {
        UGameplayStatics::OpenLevel(this, TEXT("MainMenu"), true);
    }
}

void ULobbyWidget::OnButtonHovered()
{
    if (ButtonHoverSound)
    {
        UGameplayStatics::PlaySound2D(this, ButtonHoverSound, 0.5f);
    }
}

void ULobbyWidget::OnButtonPressed()
{
    if (ButtonPressSound)
    {
        UGameplayStatics::PlaySound2D(this, ButtonPressSound, 0.7f);
    }
}

void ULobbyWidget::OnSlotInviteClickedInternal(int32 SlotIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Slot %d invite clicked"), SlotIndex);

    // Visual feedback
    if (SlotIndex < PlayerSlots.Num() && PlayerSlots[SlotIndex])
    {
        PlayerSlots[SlotIndex]->PlayInviteClickAnimation();
    }

    // Broadcast to blueprint
    OnSlotInviteClicked.Broadcast(SlotIndex);

    // Open friends UI
    OnInviteFriendsClicked();
}

void ULobbyWidget::OnSlotKickClickedInternal(int32 SlotIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Slot %d kick clicked"), SlotIndex);

    if (SlotIndex < PlayerSlots.Num() && PlayerSlots[SlotIndex] && SlotIndex > 0) // Can't kick host
    {
        FString PlayerName = PlayerSlots[SlotIndex]->GetPlayerName();
        UE_LOG(LogTemp, Warning, TEXT("TODO: Implement kick for player: %s"), *PlayerName);

        // Broadcast to blueprint for confirmation dialog
        OnSlotKickClicked.Broadcast(SlotIndex);
    }
}