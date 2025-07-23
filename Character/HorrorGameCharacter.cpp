// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorGameCharacter.h"
#include "HorrorGame/Actor/Door/Door.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGame/Widget/Settings/MenuSettingWidget.h"
#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "HorrorGame/Widget/Inventory/InventoryItem.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/Actor/MonsterJump.h"
#include "HorrorGame/Widget/Settings/DeathScreenWidget.h"
#include "HorrorGame/Widget/Progress/SanityWidget.h"
#include "HorrorGame/Widget/Item/NoteWidget.h"
#include "HorrorGame/Actor/NoteActor.h"
#include "HorrorGame/Widget/Inventory/InventoryBagWidget.h"
#include "HorrorGame/Widget/Inventory/ItemInfoWidget.h"
#include "HorrorGame/Widget/Inventory/QuantitySelectionWidget.h"
#include "HorrorGame/Widget/CrossHairWidget.h"
#include "HorrorGame/Widget/Progress/KnockOutWidget.h"
#include "HorrorGame/Actor/Door/ElectronicLockActor.h"
#include "HorrorGame/Actor/LightSwitchActor.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "HorrorGame/Actor/Door/LockedDoorActor.h"
#include "HorrorGame/Actor/Door/HospitalDoorActor.h"

// Engine
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SpotLightComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PostProcessComponent.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#include "Blueprint/WidgetBlueprintLibrary.h" 
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AHorrorGameCharacter

AHorrorGameCharacter::AHorrorGameCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Controller rotation only affects camera
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    // Character movement
    auto Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = true;
    Movement->RotationRate = FRotator(0.f, 500.f, 0.f);
    Movement->JumpZVelocity = 700.f;
    Movement->AirControl = 0.35f;
    Movement->MaxWalkSpeed = 200.f;    // Combined setting
    Movement->MinAnalogWalkSpeed = 20.f;
    Movement->BrakingDecelerationWalking = 2000.f;
    Movement->BrakingDecelerationFalling = 1500.f;

    // ===== FIRST-PERSON CAMERA (FP) =====
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetMesh(), TEXT("head"));
    CameraBoom->TargetArmLength = 0.f;    // Right at head
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraRotationLag = false;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    FollowCamera->bUsePawnControlRotation = true;
    FollowCamera->Activate();              // FP active by default

    // ===== THIRD-PERSON CAMERA (TP) =====
    ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TP_SpringArm"));
    ThirdPersonSpringArm->SetupAttachment(GetCapsuleComponent());
    ThirdPersonSpringArm->TargetArmLength = 300.f;
    ThirdPersonSpringArm->bUsePawnControlRotation = true;

    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TP_Camera"));
    ThirdPersonCamera->SetupAttachment(ThirdPersonSpringArm, USpringArmComponent::SocketName);
    ThirdPersonCamera->bUsePawnControlRotation = false;
    ThirdPersonCamera->Deactivate();             // TP off by default

    // Physics handle
    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

    // Flashlight & grabbing
    bIsFlashlightEnabled = false;
    isGrabbingObject = false;

    // Stamina
    CurrentStamina = MaxStamina = 1.f;
    StaminaSpringUsageRate = 0.1f;
    StaminaRechargeRate = 0.1f;
    CanStaminaRecharge = true;
    DelayForStaminaRecharge = 2.f;

    // Crouch state
    bIsCrouched = false;

    // Post-process
    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->SetupAttachment(GetRootComponent());
    PostProcessComponent->bEnabled = true;

    GrabbingMonster = nullptr;

    SanityTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SanityTimeline"));

    PPComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("SanityPostProcess"));
    PPComponent->SetupAttachment(GetRootComponent());
    // Đặt weight ban đầu = 0 (không ảnh hưởng)
    PPComponent->BlendWeight = 0.f;

    AnimMontages.Add(EPlayerState::PS_Death, DeathMontage);
}

void AHorrorGameCharacter::BeginPlay()
{
	Super::BeginPlay();

    SetupWidgets();

    HandleInventoryWidget();

    SetupSanityWidget();
    SetupSanityTimeline();

    Inventory.SetNum(MainInventoryCapacity);
    InventoryBag.SetNum(BagCapacity);
}

void AHorrorGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Ticks(DeltaTime);

	HandleStaminaSprint(DeltaTime);
    InitializeHeadbob();

}

//////////////////////////////////////////////////////////////////////////
// Input

void AHorrorGameCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AHorrorGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHorrorGameCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHorrorGameCharacter::Look);
		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::Interact);
		// Settings
		EnhancedInputComponent->BindAction(SettingAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleSettings);

		// Object interactions: Grab, Retrieve (attach) and Drop,....
		//EnhancedInputComponent->BindAction(HoldAction, ETriggerEvent::Triggered, this, &AHorrorGameCharacter::GrabObject);
		//EnhancedInputComponent->BindAction(HoldAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::StopGrabObject);
		EnhancedInputComponent->BindAction(InteractGrabAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::InteractWithGrabbedObject);
		EnhancedInputComponent->BindAction(Slot1Action, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleObject1);
		EnhancedInputComponent->BindAction(Slot2Action, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleObject2);
		EnhancedInputComponent->BindAction(Slot3Action, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleObject3);
		EnhancedInputComponent->BindAction(DropObjectAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::DropObject);
        EnhancedInputComponent->BindAction(ZoomObjectAction, ETriggerEvent::Triggered, this, &AHorrorGameCharacter::HandleZoom);

        //Sprint
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AHorrorGameCharacter::Sprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::UnSprint);

        //Crouch
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleCrouch);

        EnhancedInputComponent->BindAction(EscapeAAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape,EKeys::A);
        EnhancedInputComponent->BindAction(EscapeSAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape,EKeys::S);
        EnhancedInputComponent->BindAction(EscapeWAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape,EKeys::W);
        EnhancedInputComponent->BindAction(EscapeDAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape,EKeys::D);

        EnhancedInputComponent->BindAction(UpAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape, EKeys::Up);
        EnhancedInputComponent->BindAction(DownAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape, EKeys::Down);
        EnhancedInputComponent->BindAction(LeftAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape, EKeys::Left);
        EnhancedInputComponent->BindAction(RightAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::OnEscape, EKeys::Right);

        //Uses Item
        EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::UseEquippedItem); 
        
        EnhancedInputComponent->BindAction(TabAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleInventoryBag); 
        
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AHorrorGameCharacter::ToggleSettings()
{
	if (MenuSettingWidget)
	{
		MenuSettingWidget->DisplayMenu();
	}
}

void AHorrorGameCharacter::SetupWidgets()
{
    if (KnockOutWidgetClass)
    {
        KnockOutWidgetInstance = CreateWidget<UKnockOutWidget>(GetWorld(), KnockOutWidgetClass);
        if (KnockOutWidgetInstance)
        {
            // chưa add, chỉ chuẩn bị sẵn
            KnockOutWidgetInstance->AddToViewport();
            KnockOutWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
        }
    }

	if (MenuSettingWidgetClass)
	{
		MenuSettingWidget = CreateWidget<UMenuSettingWidget>(GetWorld(), MenuSettingWidgetClass);
		if (MenuSettingWidget)
		{
			MenuSettingWidget->AddToViewport();
			MenuSettingWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

    if (KeyNotificationWidgetClass)
    {
        KeyNotificationWidget = CreateWidget<UKeyNotificationWidget>(GetWorld(), KeyNotificationWidgetClass);
        if (KeyNotificationWidget)
        {
            KeyNotificationWidget->AddToViewport();
            KeyNotificationWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void AHorrorGameCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHorrorGameCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AHorrorGameCharacter::EnableFirstPerson()
{
    FollowCamera->Activate();
    CameraBoom->Activate();
    ThirdPersonCamera->Deactivate();
    ThirdPersonSpringArm->Deactivate();
    GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AHorrorGameCharacter::EnableThirdPerson()
{
    ThirdPersonSpringArm->Activate();
    ThirdPersonCamera->Activate();
    FollowCamera->Deactivate();
    CameraBoom->Deactivate();
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AHorrorGameCharacter::Interact()
{
    FHitResult HitResult;
    if (PerformInteractionLineTrace(HitResult))
    {
        AActor* HitActor = HitResult.GetActor();
        if (!HitActor) return;

        if (IInteract* Int = Cast<IInteract>(HitActor))
        {
            Int->Interact(this);
            return;
        }
    }
}

void AHorrorGameCharacter::GrabObject()
{
    FVector Start = FollowCamera->GetComponentLocation();
    FVector End = Start + FollowCamera->GetForwardVector() * InteractLineTraceLength;
    FHitResult HitResult;

    // Sử dụng line trace để lấy đối tượng
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility))
    {
        // Kiểm tra nếu đối tượng bị trúng là AItem
        if (AItem* HitItem = Cast<AItem>(HitResult.GetActor()))
        {
            // Lưu tham chiếu đối tượng AItem vào ItemRef
            ItemRef = HitItem;
            if (ItemRef->Mesh)
            {
                // Giảm InterpolationSpeed để tạo cảm giác nặng hơn
                PhysicsHandle->InterpolationSpeed = 5.0f;

                // Đảm bảo simulate physics đang được bật cho item
                if (!ItemRef->Mesh->IsSimulatingPhysics())
                {
                    ItemRef->Mesh->SetSimulatePhysics(true);
                }

                // Tính khoảng cách ban đầu giữa camera và vị trí va chạm
                CurrentGrabDistance = (HitResult.Location - Start).Size();

                // Dùng Physics Handle để bắt giữ item
                PhysicsHandle->GrabComponentAtLocationWithRotation(
                    ItemRef->Mesh,
                    NAME_None,
                    HitResult.Location,
                    FollowCamera->GetComponentRotation()
                );

                isGrabbingObject = true;
            }
        }
    }
}

void AHorrorGameCharacter::StopGrabObject()
{
    if (isGrabbingObject && PhysicsHandle)
    {
        PhysicsHandle->ReleaseComponent();
        isGrabbingObject = false;
        ItemRef = nullptr;
    }
}

void AHorrorGameCharacter::InteractWithGrabbedObject()
{
    FHitResult Hit;
    if (!PerformInteractionLineTrace(Hit))
        return;

    if (AItem* Item = Cast<AItem>(Hit.GetActor()))
    {
        ServerPickupItem(Item);
    }
}

// Counts non-null entries in a container
int32 AHorrorGameCharacter::CountValidSlots(const TArray<AActor*>& Container) const
{
    int32 Count = 0;
    for (AActor* Actor : Container)
    {
        if (Actor)
            ++Count;
    }
    return Count;
}

void AHorrorGameCharacter::HandlePickup(AItem* NewItem,TArray<AActor*>& Container,UUserWidget* InventoryUI,bool bCanGrow)
{
    if (!ensure(NewItem)) return;

    // Attempt stacking if allowed
    if (NewItem->bIsStackable)
    {
        if (TryStackIntoExisting(Container, NewItem))
        {
            RefreshUI(InventoryUI, Container);
            return;
        }
    }

    //Find slot empty
    int32 EmptyIndex = Container.IndexOfByPredicate([](AActor* Actor) { return Actor == nullptr; });
    if (EmptyIndex != INDEX_NONE)
    {
        Container[EmptyIndex] = NewItem;
    }
    else if (bCanGrow)
    {
        Container.Add(NewItem);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory full, cannot hold %s"), *NewItem->GetName());
    }

    RefreshUI(InventoryUI, Container);
    UE_LOG(LogTemp, Log, TEXT("Picked up %s into inventory."), *NewItem->GetName());
}

void AHorrorGameCharacter::RefreshUI(UUserWidget* InventoryUI, const TArray<AActor*>& Container)
{
    if (InventoryUI == nullptr)
        return;

    // Depending on type, call appropriate update method
    if (auto* MainUI = Cast<UInventory>(InventoryUI))
    {
        MainUI->UpdateInventory(Container);
    }
    else if (auto* BagUI = Cast<UInventoryBagWidget>(InventoryUI))
    {
        BagUI->UpdateBag(Container);
    }
}

void AHorrorGameCharacter::RemoveWidgetsOfClasses(std::initializer_list<TSubclassOf<UUserWidget>> WidgetClasses)
{
    for (auto WidgetClass : WidgetClasses)
    {
        TArray<UUserWidget*> FoundWidgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
            GetWorld(),
            FoundWidgets,
            WidgetClass,
            false
        );
        for (UUserWidget* W : FoundWidgets)
        {
            W->RemoveFromParent();
        }
    }
}

bool AHorrorGameCharacter::TryStackIntoExisting(TArray<AActor*>& Container, AItem* NewItem)
{
    for (AActor* Actor : Container)
    {
        if (AItem* Existing = Cast<AItem>(Actor))
        {
            if (Existing->GetClass() == NewItem->GetClass() &&
                Existing->bIsStackable &&
                Existing->Quantity < Existing->MaxStackSize)
            {
                int32 Space = Existing->MaxStackSize - Existing->Quantity;
                int32 ToMove = FMath::Min(Space, NewItem->Quantity);

                Existing->Quantity += ToMove;
                NewItem->Quantity -= ToMove;

                if (NewItem->Quantity <= 0)
                {
                    NewItem->Destroy();
                    return true;
                }
            }
        }
    }
    return false;
}

void AHorrorGameCharacter::ToggleInventoryBag()
{
    // Create widget nếu chưa có
    if (!InventoryBagWidget && InventoryBagWidgetClass)
    {
        InventoryBagWidget = CreateWidget<UInventoryBagWidget>(GetWorld(), InventoryBagWidgetClass);
    }

    if (InventoryBagWidget)
    {
        bIsBagOpen = !bIsBagOpen;
        if (bIsBagOpen)
        {
			ShowInventoryBag();
        }
        else
        {
            HideInventoryBag();

            RemoveWidgetsOfClasses({
               UItemInfoWidget::StaticClass(),
               UQuantitySelectionWidget::StaticClass()
                });
        }
    }
}

void AHorrorGameCharacter::HideInventoryBag()
{
    InventoryBagWidget->HideInventoryAnim();
    bIsBagOpen = false;
    InventoryBagWidget->RemoveFromViewport();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        PC->bShowMouseCursor = false;
    }
}

void AHorrorGameCharacter::ShowInventoryBag()
{
    bIsBagOpen = true;
	InventoryBagWidget->ShowInventoryAnim();
    InventoryBagWidget->AddToViewport();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        // Instead of UIOnly, use GameAndUI:
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(InventoryBagWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    if (UInventoryBagWidget* BagW = Cast<UInventoryBagWidget>(InventoryBagWidget))
    {
        BagW->UpdateBag(InventoryBag);
    }
}

void AHorrorGameCharacter::SwapInventoryItems(bool SourceIsBag, int32 SourceIndex, bool TargetIsBag, int32 TargetIndex)
{
    TArray<AActor*>& SourceArray = SourceIsBag ? InventoryBag : Inventory;
    TArray<AActor*>& TargetArray = TargetIsBag ? InventoryBag : Inventory;

    if (SourceArray.IsValidIndex(SourceIndex) && TargetArray.IsValidIndex(TargetIndex))
    {
        AActor* Tmp = SourceArray[SourceIndex];
        SourceArray[SourceIndex] = TargetArray[TargetIndex];
        TargetArray[TargetIndex] = Tmp;

        if (InventoryWidget)    InventoryWidget->UpdateInventory(Inventory);
        if (InventoryBagWidget) InventoryBagWidget->UpdateBag(InventoryBag);
    }
}

bool AHorrorGameCharacter::PerformInteractionLineTrace(FHitResult& OutHitResult) const
{
    if (!Actors)
    {
        return false;
    }

    const FVector Start = FollowCamera->GetComponentLocation();
    const FVector End = Start + FollowCamera->GetForwardVector() * InteractLineTraceLength;

    bool bHit = GetWorld()->LineTraceSingleByChannel(OutHitResult,Start,End,ECC_Visibility);

    return bHit && OutHitResult.GetActor() == Actors;
}

void AHorrorGameCharacter::InitializeHeadbob()
{
    const float Speed = GetVelocity().Size();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    if (Speed > 0.f && CanJump())
    {
        if (Speed < SprintSpeedThreshold)
        {
            PC->ClientStartCameraShake(WalkCameraShakeClass, 1.0f);
        }
        else
        {
            PC->ClientStartCameraShake(SprintCameraShakeClass, 1.0f);
        }
    }
    else
    {
        PC->ClientStartCameraShake(IdleCameraShakeClass, 1.0f);
    }
}

void AHorrorGameCharacter::OnEscape(const FInputActionValue& Value, FKey PressedKey)
{
    if (!GrabbingMonster) return;

    EQTEPhase Phase = GrabbingMonster->GetCurrentPhase();
    GrabbingMonster->ReceiveEscapeInput(PressedKey);
}

void AHorrorGameCharacter::RecoverSanity(float Delta)
{
    Sanity = FMath::Clamp(Sanity + Delta, 0.f, MaxSanity);
    if (SanityWidget)
        SanityWidget->SetSanityPercent(Sanity / MaxSanity);

    const float P = Sanity / MaxSanity;

    if (P > 0.6f)
    {
        // Khởi động camera shake nếu muốn
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PC->PlayerCameraManager->StartCameraShake(LowSanityShake);
        }

        // Tính alpha từ 0 → 1 khi P từ 0.6 → 1.0
        const float Alpha = (P - 0.6f) / 0.4f;

        // BlendWeight = alpha để bật dần post‑process
        PPComponent->BlendWeight = Alpha;

        // Motion Blur
        PPComponent->Settings.bOverride_MotionBlurAmount = true;
        PPComponent->Settings.MotionBlurAmount = FMath::Lerp(0.f, 1.f, Alpha);

        // Chromatic Aberration
        PPComponent->Settings.bOverride_SceneFringeIntensity = true;
        PPComponent->Settings.SceneFringeIntensity = FMath::Lerp(0.f, 5.f, Alpha);

        // Bloom
        PPComponent->Settings.bOverride_BloomIntensity = true;
        PPComponent->Settings.BloomIntensity = FMath::Lerp(0.f, 2.f, Alpha);

        // Vignette
        PPComponent->Settings.bOverride_VignetteIntensity = true;
        PPComponent->Settings.VignetteIntensity = FMath::Lerp(0.f, 0.8f, Alpha);

        // Depth of Field (F‑stop nhỏ → DOF mạnh)
        PPComponent->Settings.bOverride_DepthOfFieldFstop = true;
        PPComponent->Settings.DepthOfFieldFstop = FMath::Lerp(22.f, 1.2f, Alpha);
    }
    else
    {
        // Nếu P ≤ 0.6, tắt hoàn toàn post‑process
        PPComponent->BlendWeight = 0.f;
    }
}

void AHorrorGameCharacter::HandleDrainProgress(float Value)
{
    if (bIsGrabbed) return;

    const float DrainRate = 5.f;
    float DeltaSanity = -Value * DrainRate * GetWorld()->GetDeltaSeconds();
    RecoverSanity(DeltaSanity);
}

void AHorrorGameCharacter::SetupSanityWidget()
{
    if (SanityWidgetClass == nullptr) return;

    SanityWidget = CreateWidget<USanityWidget>(GetWorld(), SanityWidgetClass);
    if (SanityWidget)
    {
        SanityWidget->AddToViewport();
        SanityWidget->SetSanityPercent(Sanity / MaxSanity);
    }
}

void AHorrorGameCharacter::SetupSanityTimeline()
{
    if (SanityDrainCurve == nullptr || SanityTimeline == nullptr) return;

    FOnTimelineFloat ProgressDel;
    ProgressDel.BindUFunction(this, TEXT("HandleDrainProgress"));
    SanityTimeline->AddInterpFloat(SanityDrainCurve, ProgressDel);
    SanityTimeline->SetLooping(true);
    SanityTimeline->PlayFromStart();
}

void AHorrorGameCharacter::PauseSanityDrain()
{
    if (SanityTimeline && SanityTimeline->IsPlaying())
    {
        SanityTimeline->Stop();
    }
}

void AHorrorGameCharacter::ResumeSanityDrain()
{
    if (SanityTimeline && !SanityTimeline->IsPlaying())
    {
        SanityTimeline->Play();
    }
}

AActor* AHorrorGameCharacter::GetHeldObject() const
{
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    for (AActor* Actor : AttachedActors)
    {
        if (!Actor) continue;
        USceneComponent* Root = Actor->GetRootComponent();
        if (Root && Root->GetAttachParent() == GetMesh() &&
            Root->GetAttachSocketName() == FName("Object"))
        {
            return Actor;
        }
    }
    return nullptr;
}

void AHorrorGameCharacter::HandleAttachInteract(int32 Index)
{
    if (!Inventory.IsValidIndex(Index))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Inventory index: %d (Size: %d)"), Index, Inventory.Num());
        return;
    }
    if (EquippedActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already holding an object. Drop or use it before equipping a new one."));
        return;
    }

    AActor* ActorToAttach = Inventory[Index];
    if (!ActorToAttach)
    {
        UE_LOG(LogTemp, Warning, TEXT("No object at Inventory slot %d"), Index);
        return;
    }

    Inventory[Index] = nullptr;

    // Attach logic
    if (AItem* Item = Cast<AItem>(ActorToAttach))
    {
        Item->AttachToCharacter(GetMesh(), FName("Object"));
        EquippedItem = Item;
        EquippedActor = Item;
        bIsHoldingItem = true;
    }
    else
    {
        ActorToAttach->SetActorHiddenInGame(false);
        ActorToAttach->SetActorEnableCollision(false);
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(ActorToAttach->GetComponentByClass(UPrimitiveComponent::StaticClass())))
        {
            Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Prim->SetSimulatePhysics(false);
        }
        ActorToAttach->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Object"));
        EquippedActor = ActorToAttach;
        bIsHoldingItem = true;
    }

    // Keep track of slot
    EquippedIndex = Index;

    // Update UI
    OnInventoryUpdated.Broadcast(Inventory);
}

void AHorrorGameCharacter::StoreCurrentHeldObject()
{
    AActor* HeldObj = GetHeldObject();
    if (!HeldObj) return;

    // Detach
    HeldObj->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // Restore state
    if (AItem* Item = Cast<AItem>(HeldObj))
    {
        Item->OnPickup();
    }
    else
    {
        HeldObj->SetActorHiddenInGame(true);
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(HeldObj->GetComponentByClass(UPrimitiveComponent::StaticClass())))
        {
            Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Prim->SetSimulatePhysics(true);
        }
    }

    // Return to its original slot if empty, else find first nullptr, else append
    if (EquippedIndex != INDEX_NONE && Inventory.IsValidIndex(EquippedIndex) && !Inventory[EquippedIndex])
    {
        Inventory[EquippedIndex] = HeldObj;
        UE_LOG(LogTemp, Log, TEXT("Stored held object back into slot %d."), EquippedIndex);
    }
    else
    {
        int32 EmptyIdx = Inventory.IndexOfByPredicate([](AActor* Act) { return Act == nullptr; });
        if (EmptyIdx != INDEX_NONE)
        {
            Inventory[EmptyIdx] = HeldObj;
            UE_LOG(LogTemp, Log, TEXT("Stored held object into slot %d."), EmptyIdx);
        }
        else
        {
            Inventory.Add(HeldObj);
            UE_LOG(LogTemp, Log, TEXT("Inventory full, appended held object at slot %d."), Inventory.Num() - 1);
        }
    }

    // Reset equipped state
    EquippedActor = nullptr;
    EquippedItem = nullptr;
    EquippedIndex = INDEX_NONE;
    bIsHoldingItem = false;

    // UI updates
    if (InventoryWidget)
    {
        InventoryWidget->UpdateInventory(Inventory);
    }

    if (OnInventoryUpdated.IsBound())
    {
        OnInventoryUpdated.Broadcast(Inventory);
    }

    OnItemToggled.Broadcast(EquippedIndex);
}

void AHorrorGameCharacter::UseEquippedItem()
{
    if (!EquippedItem) return;

	//When the player uses an item, we check if it's a consumable or stackable item
    if (EquippedItem->bIsStackable && EquippedItem->Quantity > 1)
    {
        EquippedItem->UseItem();
        EquippedItem->Quantity--;
        Inventory[EquippedIndex] = EquippedItem;
        OnItemQuantityChanged.Broadcast(EquippedIndex);
        UE_LOG(LogTemp, Log, TEXT("Đã dùng 1, còn lại: %d"), EquippedItem->Quantity);
        return;
    }

    //When the player uses a single-use item, we remove it from the inventory
    EquippedItem->UseItem();

    StoreCurrentHeldObject();

    const int32 ClearedSlot = EquippedIndex;
    EquippedActor = nullptr;
    EquippedItem = nullptr;
    EquippedIndex = INDEX_NONE;

    OnItemToggled.Broadcast(ClearedSlot);
}


void AHorrorGameCharacter::ToggleObject(int32 Index)
{
    if (EquippedActor && EquippedIndex == Index)
    {
        StoreCurrentHeldObject();
    }
    else
    {
        if (EquippedActor)
        {
            StoreCurrentHeldObject();
        }
        HandleAttachInteract(Index);
    }
}

void AHorrorGameCharacter::RetrieveObject(int32 Index)
{
    StoreCurrentHeldObject();
    HandleAttachInteract(Index);
}

void AHorrorGameCharacter::IncreaseStat(float& CurrentValue, float MaxValue, float Amount, const FString& StatName)
{
    if (CurrentValue >= MaxValue)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s is already full. Cannot use %s item."), *StatName, *StatName);
        return;
    }

    float NewValue = FMath::Clamp(CurrentValue + Amount, 0.f, MaxValue);
    float ActualIncrease = NewValue - CurrentValue;

    if (ActualIncrease > 0.f)
    {
        CurrentValue = NewValue;
        UE_LOG(LogTemp, Log, TEXT("%s increased by %f. Current %s: %f"), *StatName, ActualIncrease, *StatName, CurrentValue);
    }
}

void AHorrorGameCharacter::IncreaseHealth(float Amount)
{
    IncreaseStat(Health, 100.f, Amount, FString("Health"));
}

FVector AHorrorGameCharacter::ComputeDropLocation(float Distance) const
{
    if (!FollowCamera)
        return GetActorLocation();

    const FVector CamLoc = FollowCamera->GetComponentLocation();
    const FVector CamFwd = FollowCamera->GetForwardVector();
    return CamLoc + CamFwd * Distance;
}

void AHorrorGameCharacter::PerformDrop(AActor* Actor, const FVector& DropLocation)
{
    if (!Actor)
        return;

    // Ensure collision
    Actor->SetActorEnableCollision(true);

    // If it's an AItem, delegate drop logic
    if (AItem* Item = Cast<AItem>(Actor))
    {
        Item->OnDrop(DropLocation);
    }
    else
    {
        // Generic actor: detach, show, simulate physics
        Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorLocation(DropLocation);

        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetComponentByClass(UPrimitiveComponent::StaticClass())))
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetSimulatePhysics(true);

            // Optional small impulse
            const FVector ImpulseDir = (FollowCamera ? FollowCamera->GetForwardVector() : GetActorForwardVector());
            PrimComp->AddImpulse(ImpulseDir * 300.f, NAME_None, true);
        }
    }
}

void AHorrorGameCharacter::DropObject()
{
    AActor* HeldObject = GetHeldObject();
    if (!HeldObject || EquippedIndex == INDEX_NONE)
        return;

    // Detach và drop
    const FVector DropLoc = ComputeDropLocation();
    PerformDrop(HeldObject, DropLoc);

    // Clear inventory slot dựa vào EquippedIndex
    Inventory[EquippedIndex] = nullptr;

    // Reset equipped
    EquippedActor = nullptr;
    EquippedItem = nullptr;
    const int32 ClearedSlot = EquippedIndex;
    EquippedIndex = INDEX_NONE;

    // Update UI
    if (InventoryWidget)
    {
        InventoryWidget->UpdateInventory(Inventory);
    }
    OnInventoryUpdated.Broadcast(Inventory);

    UE_LOG(LogTemp, Warning, TEXT("Dropped object from Inventory slot %d"), ClearedSlot);
}

void AHorrorGameCharacter::DropAllInventory()
{
    FVector DropBaseLocation = GetActorLocation() - FVector(0, 0, 50.0f); // Dưới chân nhân vật

    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        AActor* Item = Inventory[i];
        if (!Item) continue;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        FVector Offset = FVector(FMath::FRandRange(-30.f, 30.f), FMath::FRandRange(-30.f, 30.f), 0);
        FVector SpawnLocation = DropBaseLocation + Offset;
        FRotator SpawnRotation = FRotator::ZeroRotator;

        AActor* DroppedItem = GetWorld()->SpawnActor<AActor>(Item->GetClass(), SpawnLocation, SpawnRotation, SpawnParams);

        if (DroppedItem)
        {
            if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(DroppedItem->GetComponentByClass(UPrimitiveComponent::StaticClass())))
            {
                Prim->SetSimulatePhysics(true);
                Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }

    EquippedActor = nullptr;
    EquippedItem = nullptr;
    EquippedIndex = INDEX_NONE;

    if (InventoryWidget)
    {
        InventoryWidget->UpdateInventory(Inventory);
    }

    OnInventoryUpdated.Broadcast(Inventory);
}


void AHorrorGameCharacter::DropInventoryItem(bool bFromBag, int32 Index)
{
    // Choose source array
    TArray<AActor*>& SourceArray = bFromBag ? InventoryBag : Inventory;
    if (!SourceArray.IsValidIndex(Index) || !SourceArray[Index])
        return;

    AActor* DroppedActor = SourceArray[Index];

    // Clear UI immediately
    SourceArray[Index] = nullptr;
    if (InventoryWidget)    InventoryWidget->UpdateInventory(Inventory);
    if (InventoryBagWidget) InventoryBagWidget->UpdateBag(InventoryBag);

    // Drop at computed location
    const FVector DropLoc = ComputeDropLocation();
    PerformDrop(DroppedActor, DropLoc);

    UE_LOG(LogTemp, Log, TEXT("Dropped inventory item from %s slot %d"),
        bFromBag ? TEXT("Bag") : TEXT("Main"), Index);
}

void AHorrorGameCharacter::SetInventoryVisible(bool bVisible)
{
	const ESlateVisibility Visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
    if (InventoryWidget)
    {
        InventoryWidget->SetVisibility(Visibility);
	}
    if (InventorySlot)
    {
        InventoryWidget->SetVisibility(Visibility);
    }
    if (SanityWidget)
    {
        SanityWidget->SetVisibility(Visibility);
    }
}

void AHorrorGameCharacter::HandleZoom(const FInputActionValue& Value)
{
    float AxisValue = Value.Get<float>();

    if (isGrabbingObject && FMath::Abs(AxisValue) > KINDA_SMALL_NUMBER && ItemRef != nullptr)
    {
        // Cập nhật khoảng cách dựa trên input (dương => zoom in, âm => zoom out)
        CurrentGrabDistance = FMath::Clamp(
            CurrentGrabDistance + AxisValue * 20.0f,
            MinGrabDistance,
            MaxGrabDistance
        );
    }
}

void AHorrorGameCharacter::HandleInventoryWidget()
{
    if (InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UInventory>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
        }
    }

    InventoryWidget->UpdateInventory(Inventory);

    if (InventorySlotClass)
    {
        InventorySlot = CreateWidget<UInventorySlot>(GetWorld(), InventorySlotClass);
        if (InventorySlot)
        {
            InventorySlot->AddToViewport();
        }
    }
}

void AHorrorGameCharacter::Ticks(float DeltaTime)
{
    if (bIsKnockedDown && KnockOutWidgetInstance)
    {
        KnockedDownProgress = FMath::Clamp(KnockedDownProgress - AmountKnockedDownProgress * DeltaTime, 0.0f, 1.0f);
        KnockOutWidgetInstance->UpdateKnockOutProgress(KnockedDownProgress);

        if (KnockedDownProgress <= 0.0f)
        {
            bIsKnockedDown = false;
            TheChacterDeath();
            KnockOutWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (isGrabbingObject && PhysicsHandle && PhysicsHandle->GrabbedComponent)
    {
        // Lấy vị trí camera hiện tại
        FVector CameraLocation = FollowCamera->GetComponentLocation();
        // Tính vị trí mục tiêu dựa trên hướng camera và khoảng cách hiện tại
        FVector TargetLocation = CameraLocation + FollowCamera->GetForwardVector() * CurrentGrabDistance;

        // Cập nhật vị trí mục tiêu của Physics Handle
        PhysicsHandle->SetTargetLocation(TargetLocation);
    }

}

void AHorrorGameCharacter::HandleStaminaSprint(float DeltaTime)
{
    // Nếu đang crouch thì không tiêu hao stamina ngay cả khi bIsSprint == true
    if (bIsCrouched)
    {
        return;
    }

    if (bIsSprint)
    {
        // Tiêu hao stamina theo thời gian chạy
        CurrentStamina = FMath::Clamp(CurrentStamina - (StaminaSpringUsageRate * DeltaTime), 0, MaxStamina);

        if (CurrentStamina <= 0)
        {
            DepletedAllStamina();
        }
    }
    else
    {
        // Hồi phục stamina khi không chạy
        if (CurrentStamina < MaxStamina)
        {
            if (CanStaminaRecharge)
            {
                CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRechargeRate * DeltaTime), 0, MaxStamina);
            }
        }
    }
}

void AHorrorGameCharacter::EnableStaminaGain()
{
    CanStaminaRecharge = true;
}

void AHorrorGameCharacter::DepletedAllStamina()
{
    UnSprint();
}

void AHorrorGameCharacter::Sprint()
{
    if (bIsKnockedDown)
    {
        return;
    }

    if (bIsCrouched)
    {
        return;
    }

    if (GetVelocity().SizeSquared() <= 0.0f)
    {
        return;
    }

    bIsSprint = true;
    float SprintSpeed = 0.f;

    if (CurrentStamina >= 0.4f)
    {
        SprintSpeed = 600.f;
    }
    else
    {
        SprintSpeed = FMath::Lerp(200.f, 600.f, CurrentStamina / 0.4f);
    }
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

    CanStaminaRecharge = false;
    GetWorld()->GetTimerManager().ClearTimer(StaminaRechargeTimerHandle);
}

void AHorrorGameCharacter::UnSprint()
{
    if (bIsSprint)
    {
        bIsSprint = false;
        GetCharacterMovement()->MaxWalkSpeed = 200.f;

        //Start the timer to rechage stamina when the character has stopped running
        GetWorld()->GetTimerManager().SetTimer(StaminaRechargeTimerHandle, this, &AHorrorGameCharacter::EnableStaminaGain, DelayForStaminaRecharge, false);
    }
}

void AHorrorGameCharacter::ToggleCrouch()
{
    if (bIsKnockedDown) return;

	if (bIsCrouched)
	{
		UnCrouch();
        bIsCrouched = false;
	}
	else
	{
		Crouch();
        bIsCrouched = true;
	}
}

float AHorrorGameCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Health -= ActualDamage;

    if (Health <= 0.0f && !bIsKnockedDown)
    {
        StartKnockDown();
    }
    else
    {
		StopKnockDown();
    }

    return ActualDamage;
}

void AHorrorGameCharacter::StartKnockDown()
{
    bIsKnockedDown = true;
    KnockedDownProgress = 1.0f;

    EnableThirdPerson();
    SetInventoryVisible(false);
    DropAllInventory();

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 100.0f;
    }

    if (KnockOutWidgetInstance)
    {
        KnockOutWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        KnockOutWidgetInstance->NativeConstruct();
    }

	Inventory.Empty();
}

void AHorrorGameCharacter::StopKnockDown()
{
    bIsKnockedDown = false;

    EnableFirstPerson();
    SetInventoryVisible(true);

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 200.0f;
    }

    if (KnockOutWidgetInstance)
    {
        KnockOutWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
    }

}

void AHorrorGameCharacter::TheChacterDeath()
{
    PlayerState = EPlayerState::PS_Death;

    if (UAnimMontage** Found = AnimMontages.Find(PlayerState))
    {
        PlayAnimMontage(*Found);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No montage found for state %d"), PlayerState);
    }

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AHorrorGameCharacter::HandleDeath, 3.0f, false);
}

void AHorrorGameCharacter::HandleDeath()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        // Tạo widget death screen nếu lớp đã được thiết lập
        if (DeathScreenWidgetClass)
        {
            UDeathScreenWidget* DeathScreen = CreateWidget<UDeathScreenWidget>(PC, DeathScreenWidgetClass);
            if (DeathScreen)
            {
                DeathScreen->AddToViewport();
            }
        }
        PC->SetCinematicMode(true, false, false, true, true);
        PC->SetPause(true);
        PC->SetShowMouseCursor(true);
    }
}

UNoteWidget* AHorrorGameCharacter::ShowNoteUI(UTexture2D* NoteImage, const FText& NoteText)
{
    if (!NoteWidgetClass)
        return nullptr;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        NoteWidgetInstance = CreateWidget<UNoteWidget>(PC, NoteWidgetClass);
        if (NoteWidgetInstance)
        {
            NoteWidgetInstance->SetupNote(NoteImage);
            NoteWidgetInstance->AddToViewport();
            NoteWidgetInstance->SetupNoteText(NoteText);

            PC->bShowMouseCursor = true;
            UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, NoteWidgetInstance);
        }
    }

    return NoteWidgetInstance;
}

void AHorrorGameCharacter::CloseNoteUI()
{
    if (NoteWidgetInstance)
    {
        NoteWidgetInstance->RemoveFromParent();
        NoteWidgetInstance = nullptr;

        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PC->bShowMouseCursor = false;
            UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
            EnableInput(PC);
        }
    }

    // Nếu đang giữ NoteActor (CurrentNote), trả về vị trí cũ rồi mở tiếp UI khóa
    if (CurrentNote)
    {
        // Lưu lại con trỏ trước khi null
        AActor* NoteActor = CurrentNote;

        // Trả note về vị trí ban đầu
        CurrentNote->ReturnToOriginal();
        CurrentNote = nullptr;

        // Nếu đó là ElectronicLockActor, gọi Interact để show mã nhập
        if (AElectronicLockActor* Lock = Cast<AElectronicLockActor>(NoteActor))
        {
            Lock->Interact(this);
        }
    }
}

bool AHorrorGameCharacter::ServerInteract_Validate(AInteractableActor* Target)
{
    return Target != nullptr;
}

void AHorrorGameCharacter::ServerInteract_Implementation(AInteractableActor* Target)
{
    if (!Target) return;

    if (auto Switch = Cast<ALightSwitchActor>(Target))
    {
        Switch->MulticastToggleLightSwitch();
    }

    if (auto Door = Cast<ALockedDoorActor>(Target))
    {
        Door->ServerInteract(this);
    }
    
    if (auto HospitalDoor = Cast<AHospitalDoorActor>(Target))
    {
        HospitalDoor->ServerInteract(this);
    }
}

bool AHorrorGameCharacter::ServerPickupItem_Validate(AItem* Item)
{
    return Item != nullptr;
}

void AHorrorGameCharacter::ServerPickupItem_Implementation(AItem* Item)
{
    if (!Item || !Item->HasAuthority()) return;

    // 1) Ẩn / destroy trên server
    Item->OnPickup();

    // 2) Multicast để client ẩn
    Item->MulticastOnPickedUp();

    // 3) Add vào inventory server‑side
    int32 MainCount = 0;
    for (auto* I : Inventory) if (I) ++MainCount;

    if (MainCount < MainInventoryCapacity)
    {
        HandlePickup(Item, Inventory, InventoryWidget, /*bCanGrow=*/ true);
        RefreshUI(InventoryWidget, Inventory);
    }
    else
    {
        HandlePickup(Item, InventoryBag, InventoryBagWidget, /*bCanGrow=*/ false);
        RefreshUI(InventoryBagWidget, InventoryBag);
    }
}

void AHorrorGameCharacter::OnRep_Inventory()
{
    OnInventoryUpdated.Broadcast(Inventory);
}

void AHorrorGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHorrorGameCharacter, Inventory);
	DOREPLIFETIME(AHorrorGameCharacter, InventoryBag);
	DOREPLIFETIME(AHorrorGameCharacter, EquippedActor);
	DOREPLIFETIME(AHorrorGameCharacter, EquippedItem);
	DOREPLIFETIME(AHorrorGameCharacter, Health);
	DOREPLIFETIME(AHorrorGameCharacter, CurrentStamina);
	DOREPLIFETIME(AHorrorGameCharacter, bIsSprint);
	DOREPLIFETIME(AHorrorGameCharacter, bIsFlashlightEnabled);
}