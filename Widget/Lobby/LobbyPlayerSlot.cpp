#include "LobbyPlayerSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Engine/Texture2D.h"

void ULobbyPlayerSlot::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (InviteButton)
    {
        InviteButton->OnClicked.AddDynamic(this, &ULobbyPlayerSlot::OnInviteButtonClicked);
    }

    if (KickButton)
    {
        KickButton->OnClicked.AddDynamic(this, &ULobbyPlayerSlot::OnKickButtonClicked);
    }

    // Initialize as empty slot
    SetEmpty();
}

void ULobbyPlayerSlot::SetDynamicMaterials(
    UMaterialInstanceDynamic* HostMat,
    UMaterialInstanceDynamic* NormalMat,
    UMaterialInstanceDynamic* EmptyMat)
{
    HostBorderMaterialDynamic = HostMat;
    NormalBorderMaterialDynamic = NormalMat;
    EmptyBorderMaterialDynamic = EmptyMat;

    // Apply initial material
    ApplyMaterialStyling();
}

void ULobbyPlayerSlot::SetPlayerInfo(const FString& PlayerName, bool bInIsHost)
{
    // Store player data
    CurrentPlayerName = PlayerName;
    bIsOccupied = true;
    bIsHost = bInIsHost;

    // Update UI immediately
    UpdateSlotContent();
    ApplyMaterialStyling();
}

void ULobbyPlayerSlot::SetPlayerInfoWithAnimation(const FString& PlayerName, bool bInIsHost)
{
    // Store new info
    CurrentPlayerName = PlayerName;
    bIsOccupied = true;
    bIsHost = bInIsHost;

    // Play join animation
    PlaySlotJoinAnimation();

    TWeakObjectPtr<ULobbyPlayerSlot> Weakthis(this);
    FTimerHandle UpdateTimer;
    GetWorld()->GetTimerManager().SetTimer(UpdateTimer, [Weakthis]()
        {
            if (ULobbyPlayerSlot* PlayerSlot = Weakthis.Get())
            {
                PlayerSlot->UpdateSlotContent();
                PlayerSlot->ApplyMaterialStyling();

                // Start host glow if this is a host
                if (PlayerSlot->bIsHost)
                {
                    PlayerSlot->PlayHostGlowAnimation();
                }
            }
        }, 0.2f, false);
}

void ULobbyPlayerSlot::SetEmpty()
{
    // Clear player data
    CurrentPlayerName = TEXT("");
    bIsOccupied = false;
    bIsHost = false;

    // Update UI for empty state
    UpdateSlotContent();
    ApplyMaterialStyling();
}

void ULobbyPlayerSlot::UpdateSlotContent()
{
    if (!IsValid(PlayerNameText)) return;

    // Update player name
    if (PlayerNameText)
    {
        if (bIsOccupied)
        {
            PlayerNameText->SetText(FText::FromString(CurrentPlayerName));
            PlayerNameText->SetColorAndOpacity(bIsHost ? FLinearColor::Yellow : FLinearColor::White);
            // Enhanced styling for host
            if (bIsHost)
            {
                FSlateFontInfo FontInfo = PlayerNameText->GetFont();
                FontInfo.Size = 20; 
                PlayerNameText->SetFont(FontInfo);
            }
            else
            {
                FSlateFontInfo FontInfo = PlayerNameText->GetFont();
                FontInfo.Size = 18;
                PlayerNameText->SetFont(FontInfo);
            }
        }
        else
        {
            PlayerNameText->SetText(FText::FromString(TEXT("Empty Slot")));
            PlayerNameText->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
            FSlateFontInfo FontInfo = PlayerNameText->GetFont();
            FontInfo.Size = 16;
            PlayerNameText->SetFont(FontInfo);
        }
    }

    // Update status text
    if (StatusText)
    {
        if (bIsOccupied)
        {
            FString StatusString = bIsHost ? TEXT("HOST") : TEXT("SURVIVOR");
            StatusText->SetText(FText::FromString(StatusString));
            StatusText->SetColorAndOpacity(bIsHost ? FLinearColor::Yellow : FLinearColor::Gray);
        }
        else
        {
            StatusText->SetText(FText::FromString(TEXT("Waiting for player...")));
            StatusText->SetColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
        }
    }

    // Update host crown visibility
    if (HostIcon)
    {
        if (bIsHost && bIsOccupied)
        {
            HostIcon->SetVisibility(ESlateVisibility::Visible);
            HostIcon->SetRenderScale(FVector2D(0.5f, 0.5f));
            HostIcon->SetColorAndOpacity(FLinearColor::Yellow);

            // Dùng TWeakObjectPtr để capture HostIcon an toàn
            TWeakObjectPtr<UImage> WeakHostIcon = HostIcon;
            TWeakObjectPtr<ULobbyPlayerSlot> WeakThis(this);
            auto ScaleTime = MakeShared<float>(0.0f);


            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(
                    CrownTimer,
                    [WeakThis, WeakHostIcon, ScaleTime]()
                    {
                        // Nếu widget hoặc icon đã hủy thì clear timer
                        if (!WeakThis.IsValid() || !WeakHostIcon.IsValid())
                        {
                            if (UWorld* W = WeakThis.IsValid() ? WeakThis->GetWorld() : nullptr)
                            {
                                W->GetTimerManager().ClearTimer(WeakThis->CrownTimer);
                            }
                            return;
                        }

                        *ScaleTime += 0.016f;
                        float Progress = FMath::Clamp(*ScaleTime / 0.3f, 0.0f, 1.0f);
                        float Scale = FMath::Lerp(0.5f, 1.0f, FMath::SmoothStep(0.0f, 1.0f, Progress));

                        WeakHostIcon->SetRenderScale(FVector2D(Scale, Scale));

                        if (Progress >= 1.0f)
                        {
                            if (UWorld* W = WeakThis->GetWorld())
                            {
                                W->GetTimerManager().ClearTimer(WeakThis->CrownTimer);
                            }
                        }
                    },
                    0.016f,
                    true
                );
            }
        }
        else
        {
            HostIcon->SetVisibility(ESlateVisibility::Hidden);
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().ClearTimer(CrownTimer);
            }
        }
    }

    // Update player avatar
    if (PlayerAvatar)
    {
        if (bIsOccupied)
        {
            // Set player avatar or default
            PlayerAvatar->SetColorAndOpacity(FLinearColor::White);
            // Create simple avatar with player initial
            if (!CurrentPlayerName.IsEmpty())
            {
                // You could set a texture here or create a simple colored circle
                // For now, we'll use color coding
                FLinearColor AvatarColor = bIsHost ?
                    FLinearColor::Yellow :
                    FLinearColor(0.3f, 0.6f, 1.0f, 1.0f); // Blue for players
                PlayerAvatar->SetColorAndOpacity(AvatarColor);
            }
        }
        else
        {
            // Empty slot avatar
            PlayerAvatar->SetColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
        }
    }

    // Update button visibility
    UpdateButtonVisibility();
}

void ULobbyPlayerSlot::UpdateButtonVisibility()
{
    // Invite button: visible only for empty slots
    if (InviteButton)
    {
        InviteButton->SetVisibility(bIsOccupied ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

        if (!bIsOccupied)
        {
            // Style invite button
            InviteButton->SetToolTipText(FText::FromString(TEXT("Invite a friend to this slot")));
        }
    }

    // Kick button: visible only for occupied non-host slots
    if (KickButton)
    {
        bool bShowKick = bIsOccupied && !bIsHost && SlotIndex > 0; // Can't kick host or slot 0
        KickButton->SetVisibility(bShowKick ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

        if (bShowKick)
        {
            FString ToolTip = FString::Printf(TEXT("Kick %s from the lobby"), *CurrentPlayerName);
            KickButton->SetToolTipText(FText::FromString(ToolTip));
        }
    }
}

void ULobbyPlayerSlot::ApplyMaterialStyling()
{
    if (!SlotBorder) return;

    UMaterialInstanceDynamic* TargetMaterial = nullptr;

    // Choose appropriate material based on slot state
    if (!bIsOccupied)
    {
        TargetMaterial = EmptyBorderMaterialDynamic;
    }
    else if (bIsHost)
    {
        TargetMaterial = HostBorderMaterialDynamic;
    }
    else
    {
        TargetMaterial = NormalBorderMaterialDynamic;
    }

    if (TargetMaterial)
    {
        // Apply material to border
        SlotBorder->SetBrushFromMaterial(TargetMaterial);

        // Set material parameters for different states
        if (bIsHost && bIsOccupied)
        {
            // Host styling with gold glow
            TargetMaterial->SetScalarParameterValue(TEXT("GlowIntensity"), 0.8f);
            TargetMaterial->SetVectorParameterValue(TEXT("BorderColor"), FLinearColor::Yellow);
            TargetMaterial->SetScalarParameterValue(TEXT("BorderWidth"), 3.0f);
        }
        else if (bIsOccupied)
        {
            // Regular player styling
            TargetMaterial->SetScalarParameterValue(TEXT("GlowIntensity"), 0.4f);
            TargetMaterial->SetVectorParameterValue(TEXT("BorderColor"), FLinearColor::White);
            TargetMaterial->SetScalarParameterValue(TEXT("BorderWidth"), 2.0f);
        }
        else
        {
            // Empty slot styling with pulse
            TargetMaterial->SetScalarParameterValue(TEXT("GlowIntensity"), 0.2f);
            TargetMaterial->SetVectorParameterValue(TEXT("BorderColor"), FLinearColor::Gray);
            TargetMaterial->SetScalarParameterValue(TEXT("PulseSpeed"), 1.5f);
            TargetMaterial->SetScalarParameterValue(TEXT("BorderWidth"), 1.0f);
        }
    }
}

void ULobbyPlayerSlot::SetHostStyling(bool bInIsHost)
{
    bIsHost = bInIsHost;

    if (bIsHost && bIsOccupied)
    {
        // Enhanced host styling
        if (PlayerNameText)
        {
            PlayerNameText->SetColorAndOpacity(FLinearColor::Yellow);
        }

        if (StatusText)
        {
            StatusText->SetText(FText::FromString(TEXT("HOST")));
            StatusText->SetColorAndOpacity(FLinearColor::Yellow);
        }

        if (HostIcon)
        {
            HostIcon->SetVisibility(ESlateVisibility::Visible);
        }

        // Apply host material
        ApplyMaterialStyling();

        // Start continuous glow animation
        PlayHostGlowAnimation();
    }
}

// Button Event Handlers
void ULobbyPlayerSlot::OnInviteButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Invite button clicked for slot %d"), SlotIndex);

    // Play click animation
    PlayInviteClickAnimation();

    // Broadcast to parent widget
    OnInviteClicked.Broadcast(SlotIndex);
}

void ULobbyPlayerSlot::OnKickButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Kick button clicked for slot %d - Player: %s"), SlotIndex, *CurrentPlayerName);

    // Only allow kicking if not host and slot is occupied
    if (bIsOccupied && !bIsHost && SlotIndex > 0)
    {
        // Broadcast to parent widget
        OnKickClicked.Broadcast(SlotIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot kick this player"));
    }
}