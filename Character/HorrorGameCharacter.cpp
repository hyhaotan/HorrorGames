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
#include "HorrorGame/Widget/Item/NoteWidget.h"
#include "HorrorGame/Actor/NoteActor.h"
#include "HorrorGame/Widget/CrossHairWidget.h"
#include "HorrorGame/Widget/Progress/KnockOutWidget.h"
#include "HorrorGame/Actor/Door/ElectronicLockActor.h"
#include "HorrorGame/Actor/Light/LightSwitchActor.h"
#include "HorrorGame/Widget/KeyNotificationWidget.h"
#include "HorrorGame/Actor/Door/LockedDoorActor.h"
#include "HorrorGame/Actor/Door/HospitalDoorActor.h"
#include "HorrorGame/AI/NPC.h"
#include "HorrorGame/Actor/Component/SprintComponent.h"
#include "HorrorGame/Actor/Component/HeadbobComponent.h"
#include "HorrorGame/Actor/Component/SanityComponent.h"
#include "HorrorGame/Actor/Component/FearComponent.h"
#include "HorrorGame/Widget/Lobby/LobbyPlayerSlot.h"

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
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AHorrorGameCharacter

AHorrorGameCharacter::AHorrorGameCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);
    SetReplicates(true);

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

    // Crouch state
    bIsCrouched = false;

    // Post-process
    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->SetupAttachment(GetRootComponent());
    PostProcessComponent->bEnabled = true;

    GrabbingMonster = nullptr;

    PPComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("SanityPostProcess"));
    PPComponent->SetupAttachment(GetRootComponent());
    // Đặt weight ban đầu = 0 (không ảnh hưởng)
    PPComponent->BlendWeight = 0.f;

    AnimMontages.Add(EPlayerState::PS_Death, DeathMontage);

	//Components
	SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
    HeadbobComponent = CreateDefaultSubobject<UHeadbobComponent>(TEXT("HeadbobComponent"));
	SanityComponent = CreateDefaultSubobject<USanityComponent>(TEXT("SanityComponent"));
    FearComponent = CreateDefaultSubobject<UFearComponent>(TEXT("FearComponent"));

    bIsBeingChased = false;
    DarknessVolumeCount = 0;

    LobbySlotWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("LobbySlotWidget"));
    LobbySlotWidgetComp->SetupAttachment(GetMesh(), TEXT("head")); 
    LobbySlotWidgetComp->SetDrawSize(FVector2D(200, 50));
    LobbySlotWidgetComp->SetRelativeLocation(FVector(0, 0, 30));
    LobbySlotWidgetComp->SetWidgetClass(LobbySlotWidgetClass);

}

void AHorrorGameCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (HeadbobComponent && SanityComponent)
    {
        HeadbobComponent->OnHeadbobStateChanged.AddDynamic(this, &AHorrorGameCharacter::HandleHeadbobStateChanged);
        HeadbobComponent->ForceUpdateHeadbob();
        SanityComponent->OnSanityThresholdReached.AddDynamic(this, &AHorrorGameCharacter::OnSanityThresholdReached);
    }

    if (UUserWidget* BaseWidget = LobbySlotWidgetComp->GetUserWidgetObject())
    {
        if (ULobbyPlayerSlot* SlotWidget = Cast<ULobbyPlayerSlot>(BaseWidget))
        {
            // Lấy PlayerState hoặc Controller để đặt tên, trạng thái
            FString PlayerName = GetPlayerState() ? GetPlayerState()->GetPlayerName() : TEXT("Guest");
            bool bIsReady = false;

            SlotWidget->SetPlayerInfo(PlayerName, bIsReady);
        }
    }


    SetupWidgets();

    HandleInventoryWidget();

    Inventory.SetNum(MainInventoryCapacity);
}

void AHorrorGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Ticks(DeltaTime);
}

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

    if (IsLocallyControlled() && !KnockOutWidgetInstance)
    {
        // Thay YourKnockOutWidgetClass bằng class thực tế của bạn
        if (KnockOutWidgetClass)
        {
            KnockOutWidgetInstance = CreateWidget<UKnockOutWidget>(GetWorld(), KnockOutWidgetClass);
            if (KnockOutWidgetInstance)
            {
                KnockOutWidgetInstance->AddToViewport();
                KnockOutWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
                UE_LOG(LogTemp, Warning, TEXT("InitializeKnockOutWidget: Widget created and added to viewport"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("InitializeKnockOutWidget: Failed to create widget instance"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InitializeKnockOutWidget: KnockOutWidgetClass is NULL"));
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

void AHorrorGameCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (HeadbobComponent)
    {
        // Ensure delegate is bound in case binding moved here
        HeadbobComponent->OnHeadbobStateChanged.AddDynamic(this, &AHorrorGameCharacter::HandleHeadbobStateChanged);
    }
}

void AHorrorGameCharacter::HandleHeadbobStateChanged(EHeadbobState NewState, EHeadbobState OldState)
{
    switch (NewState)
    {
    case EHeadbobState::Sprinting:
        // Play sprint sound, breathing effects, etc.
        break;
    case EHeadbobState::Walking:
        // Play walk sound effects
        break;
    case EHeadbobState::Crouching:
        // Play crouch movement sounds
        break;
    case EHeadbobState::None:
        // Stop movement sounds
        break;
    case EHeadbobState::Custom:
        // Custom effects
        break;
    default:
        break;
    }

    // Notify blueprints
    OnHeadbobStateChanged(NewState, OldState);
}

void AHorrorGameCharacter::EnableHeadbob(bool bEnable)
{
    if (HeadbobComponent)
    {
        HeadbobComponent->SetHeadbobEnabled(bEnable);
    }
}

void AHorrorGameCharacter::SetCustomHeadbob(TSubclassOf<UCameraShakeBase> CustomShake, float Intensity)
{
    if (HeadbobComponent)
    {
        HeadbobComponent->SetCustomHeadbob(CustomShake, Intensity);
    }
}


void AHorrorGameCharacter::OnEscape(const FInputActionValue& Value, FKey PressedKey)
{
    if (!GrabbingMonster) return;

    EQTEPhase Phase = GrabbingMonster->GetCurrentPhase();
    GrabbingMonster->ReceiveEscapeInput(PressedKey);
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

void AHorrorGameCharacter::HandlePickup(AItem* NewItem, TArray<AActor*>& Container, UUserWidget* InventoryUI, bool bCanGrow)
{
    if (!ensure(NewItem)) return;

    int32 CurrentItemCount = CountValidSlots(Container);

    if (CurrentItemCount >= 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory full (3/3), cannot pick up %s"), *NewItem->GetName());
        return;
    }

    int32 EmptyIndex = Container.IndexOfByPredicate([](AActor* Actor) { return Actor == nullptr; });

    if (EmptyIndex != INDEX_NONE)
    {
        Container[EmptyIndex] = NewItem;
        UE_LOG(LogTemp, Log, TEXT("Picked up %s into slot %d. (%d/3)"), *NewItem->GetName(), EmptyIndex, CurrentItemCount + 1);
    }
    else if (bCanGrow && Container.Num() < 3)
    {
        Container.Add(NewItem);
        UE_LOG(LogTemp, Log, TEXT("Picked up %s by growing inventory. (%d/3)"), *NewItem->GetName(), CurrentItemCount + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory full or cannot grow, cannot pick up %s"), *NewItem->GetName());
        return;
    }

    RefreshUI(InventoryUI, Container);
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
    TArray<AActor*>& SourceArray = Inventory;
    if (!SourceArray.IsValidIndex(Index) || !SourceArray[Index])
        return;

    AActor* DroppedActor = SourceArray[Index];

    // Clear UI immediately
    SourceArray[Index] = nullptr;
    if (InventoryWidget)    InventoryWidget->UpdateInventory(Inventory);

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

void AHorrorGameCharacter::Sprint()
{
    SprintComponent->StartSprint();
}

void AHorrorGameCharacter::UnSprint()
{
    SprintComponent->StopSprint();
}

void AHorrorGameCharacter::ClientStartJumpScare_Implementation(ANPC* JumpscareNPC, float BlendTime, float Duration)
{
    if (!JumpscareNPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("ClientStartJumpScare: JumpscareNPC is null"));
        return;
    }

    // Chỉ thực hiện trên client của player này
    if (!IsLocallyControlled())
    {
        return;
    }

    APlayerController* PC = GetController<APlayerController>();
    if (!PC)
    {
        return;
    }

    this->DisableInput(PC);

    if (SanityComponent)
    {
        SanityComponent->OnJumpScare(0.15f);
    }

    // Lưu view target hiện tại
    OriginalViewTarget = PC->GetViewTarget();

    // Bật cờ để NPC sử dụng jumpscare camera
    JumpscareNPC->bUseJumpScareCamera = true;

    // Chuyển camera sang NPC
    PC->SetViewTargetWithBlend(JumpscareNPC, BlendTime, EViewTargetBlendFunction::VTBlend_Cubic);

    // Set timer để kết thúc jumpscare
    GetWorld()->GetTimerManager().SetTimer(
        JumpscareTimerHandle,
        this,
        &AHorrorGameCharacter::OnJumpscareCameraComplete,
        Duration,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("ClientStartJumpScare: Started jumpscare for player %s"), *GetName());
}

void AHorrorGameCharacter::ClientEndJumpScare_Implementation(float BlendTime)
{
    // Chỉ thực hiện trên client của player này
    if (!IsLocallyControlled())
    {
        return;
    }

    OnJumpscareCameraComplete();
}

void AHorrorGameCharacter::OnJumpscareCameraComplete()
{
    // Clear timer
    GetWorld()->GetTimerManager().ClearTimer(JumpscareTimerHandle);

    APlayerController* PC = GetController<APlayerController>();
    if (!PC)
    {
        return;
    }

	this->EnableInput(PC);

    // Chuyển camera về player
    if (OriginalViewTarget)
    {
        PC->SetViewTargetWithBlend(OriginalViewTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
    }
    else
    {
        PC->SetViewTargetWithBlend(this, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
    }

    OriginalViewTarget = nullptr;

    UE_LOG(LogTemp, Log, TEXT("OnJumpscareCameraComplete: Restored camera for player %s"), *GetName());
}

void AHorrorGameCharacter::ServerStartKnockDown_Implementation()
{
    // Chỉ thực hiện trên server
    if (!HasAuthority())
    {
        return;
    }

    StartKnockDown();
}

void AHorrorGameCharacter::ServerStopKnockDown_Implementation()
{
    // Chỉ thực hiện trên server
    if (!HasAuthority())
    {
        return;
    }

    StopKnockDown();
}

void AHorrorGameCharacter::StartKnockDown()
{
    // Nếu đang chạy trên client, gọi server
    if (!HasAuthority())
    {
        ServerStartKnockDown();
        return;
    }

    // Logic knockdown (chạy trên server, sẽ replicate)
    bIsKnockedDown = true;
    KnockedDownProgress = 1.0f;

    // Các thay đổi này sẽ được thực hiện trên tất cả clients thông qua OnRep
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 100.0f;
    }

    // Empty inventory trên server
    Inventory.Empty();

    UE_LOG(LogTemp, Log, TEXT("StartKnockDown: Player %s knocked down"), *GetName());
}

void AHorrorGameCharacter::StopKnockDown()
{
    // Nếu đang chạy trên client, gọi server
    if (!HasAuthority())
    {
        ServerStopKnockDown();
        return;
    }

    // Logic stop knockdown (chạy trên server, sẽ replicate)
    bIsKnockedDown = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 200.0f;
    }

    UE_LOG(LogTemp, Log, TEXT("StopKnockDown: Player %s recovered"), *GetName());
}

void AHorrorGameCharacter::OnRep_IsKnockedDown()
{
    // Hàm này chạy trên tất cả clients khi bIsKnockedDown thay đổi
    if (bIsKnockedDown)
    {
        // Knockdown effects chỉ áp dụng cho local player
        if (IsLocallyControlled())
        {
            EnableThirdPerson();
            SetInventoryVisible(false);
            DropAllInventory();

            if (KnockOutWidgetInstance)
            {
                KnockOutWidgetInstance->SetVisibility(ESlateVisibility::Visible);
                KnockOutWidgetInstance->NativeConstruct();
            }
        }
    }
    else
    {
        // Recovery effects chỉ áp dụng cho local player
        if (IsLocallyControlled())
        {
            EnableFirstPerson();
            SetInventoryVisible(true);

            if (KnockOutWidgetInstance)
            {
                KnockOutWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("OnRep_IsKnockedDown: Player %s knockdown state: %s"),
        *GetName(), bIsKnockedDown ? TEXT("TRUE") : TEXT("FALSE"));
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

    // Handle light switches
    if (auto Switch = Cast<ALightSwitchActor>(Target))
    {
        Switch->MulticastToggleLightSwitch();
    }

    // Handle all door types using the base DoorRootActor
    if (auto Door = Cast<ADoorRootActor>(Target))
    {
        Door->ServerDoorInteract(this);
    }

    // Legacy specific door handling (can be removed if using DoorRootActor base)
    /*
    if (auto LockedDoor = Cast<ALockedDoorActor>(Target))
    {
        LockedDoor->ServerInteract(this);
    }

    if (auto HospitalDoor = Cast<AHospitalDoorActor>(Target))
    {
        HospitalDoor->ServerInteract(this);
    }
     */
}

bool AHorrorGameCharacter::ServerPickupItem_Validate(AItem* Item)
{
    return Item != nullptr;
}

void AHorrorGameCharacter::ServerPickupItem_Implementation(AItem* Item)
{
    if (!Item || !Item->HasAuthority()) return;

    int32 MainCount = CountValidSlots(Inventory);
    if (MainCount >= 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server: Inventory full (3/3), cannot pick up %s"), *Item->GetName());
        return; 
    }

    Item->OnPickup();

    Item->MulticastOnPickedUp();

    HandlePickup(Item, Inventory, InventoryWidget, true);
}

void AHorrorGameCharacter::OnRep_Inventory()
{
    OnInventoryUpdated.Broadcast(Inventory);
}

void AHorrorGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHorrorGameCharacter, Inventory);
	DOREPLIFETIME(AHorrorGameCharacter, EquippedActor);
	DOREPLIFETIME(AHorrorGameCharacter, EquippedItem);
	DOREPLIFETIME(AHorrorGameCharacter, bIsFlashlightEnabled);
	DOREPLIFETIME(AHorrorGameCharacter, bIsKnockedDown);
}

float AHorrorGameCharacter::GetSanityLevel() const
{
    return SanityComponent ? SanityComponent->GetSanityPercent() : 1.0f;
}

float AHorrorGameCharacter::GetFearLevel() const
{
    if (FearComponent)
    {
        return FearComponent->GetFearLevel();
    }

    // Fallback: tính toán fear level phức tạp hơn
    float BaseFear = 1.0f - GetSanityLevel(); // Sanity thấp = fear cao

    // Multipliers dựa trên các yếu tố môi trường
    float EnvironmentalFear = 0.0f;

    // Darkness fear (tăng dần theo thời gian trong bóng tối)
    if (IsInDarkness())
    {
        EnvironmentalFear += FMath::Min(0.3f, DarknessExposureTime * 0.05f);
    }

    // Chase fear (rất cao và giảm dần)
    if (IsBeingChased())
    {
        float ChaseTime = GetWorld()->GetTimeSeconds() - LastChaseTime;
        float ChaseFear = FMath::Lerp(0.4f, 0.1f, ChaseTime / ChaseMemoryTime);
        EnvironmentalFear += ChaseFear;
    }

    // Threat proximity fear
    if (IsInThreatProximity())
    {
        EnvironmentalFear += ThreatProximityAmount * 0.2f;
    }

    // Monster proximity fear (khoảng cách càng gần càng sợ)
    if (NearestMonsterDistance > 0.0f && NearestMonsterDistance < MaxFearDistance)
    {
        float ProximityFear = 1.0f - (NearestMonsterDistance / MaxFearDistance);
        EnvironmentalFear += ProximityFear * 0.25f;
    }

    // Combine base fear với environmental fear
    float TotalFear = BaseFear + EnvironmentalFear;

    // Sanity state multiplier
    if (SanityComponent)
    {
        ESanityState SanityState = SanityComponent->GetSanityState();
        switch (SanityState)
        {
        case ESanityState::Critical:
            TotalFear *= 1.5f;
            break;
        case ESanityState::Unstable:
            TotalFear *= 1.3f;
            break;
        case ESanityState::Anxious:
            TotalFear *= 1.1f;
            break;
        default:
            break;
        }
    }

    return FMath::Clamp(TotalFear, 0.0f, 1.0f);
}

void AHorrorGameCharacter::OnSanityThresholdReached(float ThresholdPercent)
{
    // Cập nhật headbob component
    if (UHeadbobComponent* HB = FindComponentByClass<UHeadbobComponent>())
    {
        HB->ForceUpdateHeadbob();
    }

    // Trigger different effects based on threshold
    if (ThresholdPercent <= 0.8f && ThresholdPercent > 0.6f)
    {
        // Nervousness threshold
        UGameplayStatics::PlaySound2D(this, NervousBreathingSound, 0.3f);
    }
    else if (ThresholdPercent <= 0.6f && ThresholdPercent > 0.4f)
    {
        // Anxiety threshold
        UGameplayStatics::PlaySound2D(this, AnxiousWhisperSound, 0.5f);

        // Slight movement speed reduction
        GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.9f;
    }
    else if (ThresholdPercent <= 0.4f && ThresholdPercent > 0.2f)
    {
        // Unstable threshold
        UGameplayStatics::PlaySound2D(this, UnstableSound, 0.7f);

        // More significant movement impairment
        GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.8f;

        // Chance for screen distortion
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            /*TriggerScreenDistortion(2.0f);*/
        }
    }
    else if (ThresholdPercent <= 0.2f)
    {
        // Critical threshold
        UGameplayStatics::PlaySound2D(this, CriticalPanicSound, 1.0f);

        // Severe movement impairment
        GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.6f;

        // Trigger post-process effects
        if (SanityComponent)
        {
            SanityComponent->TriggerLowSanityPostProcess();
        }

        // High chance of hallucination
        if (FMath::RandRange(0.0f, 1.0f) < 0.6f)
        {
            TriggerRandomHallucination();
        }
    }

    // Blueprint event
    BP_OnSanityThresholdReached(ThresholdPercent);

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sanity threshold reached: %.1f%% - Effects applied"),
            ThresholdPercent * 100.0f);
    }
}

void AHorrorGameCharacter::OnPanicAttack()
{
    // Temporary severe movement impairment
    GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.3f;

    // Intense screen shake
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PanicCameraShake)
        {
            PC->PlayerCameraManager->StartCameraShake(PanicCameraShake, 2.0f);
        }
    }

    // Play panic audio
    UGameplayStatics::PlaySound2D(this, PanicAttackSound, 1.0f);

    // Restore movement after panic duration
    FTimerHandle RestoreMovementTimer;
    GetWorld()->GetTimerManager().SetTimer(
        RestoreMovementTimer,
        [this]()
        {
            GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed *
                (SanityComponent ? FMath::Lerp(0.6f, 1.0f, SanityComponent->GetSanityPercent()) : 1.0f);
        },
        PanicDuration,
        false
    );

    BP_OnPanicAttack();
}

void AHorrorGameCharacter::OnHallucination()
{
    TriggerRandomHallucination();
    BP_OnHallucination();
}

void AHorrorGameCharacter::TriggerRandomHallucination()
{
    if (HallucinationTypes.Num() == 0) return;

    int32 RandomIndex = FMath::RandRange(0, HallucinationTypes.Num() - 1);
    EHallucinationType HType = HallucinationTypes[RandomIndex];

    switch (HType)
    {
    case EHallucinationType::FakeMonster:
        break;

    case EHallucinationType::FalseSound:
        PlayFalseSound();
        break;

    case EHallucinationType::WallMovement:
        TriggerWallMovementHallucination();
        break;

    case EHallucinationType::ShadowPeople:
        break;

    case EHallucinationType::FakeExit:
        break;
    }

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Warning, TEXT("Hallucination triggered: %d"), (int32)HType);
    }
}

bool AHorrorGameCharacter::IsBeingChased() const
{
    return bIsBeingChased;
}

bool AHorrorGameCharacter::IsInDarkness() const
{
    return DarknessVolumeCount > 0 || CurrentLightLevel < DarknessThreshold;
}

bool AHorrorGameCharacter::IsInThreatProximity() const
{
    return ThreatProximityAmount > 0.0f || NearestMonsterDistance < ThreatProximityDistance;
}

void AHorrorGameCharacter::NotifyChasedByMonster(float ChaseIntensity)
{
    bIsBeingChased = true;
    LastChaseTime = GetWorld()->GetTimeSeconds();
    CurrentChaseIntensity = ChaseIntensity;

    // Immediate sanity loss when chase begins
    if (SanityComponent)
    {
        float ChaseSanityLoss = ChaseIntensity * 5.0f; // Scaled by intensity
        SanityComponent->DrainSanity(ChaseSanityLoss, true);
    }

    // Clear and reset chase memory timer
    GetWorldTimerManager().ClearTimer(TimerHandle_ResetChase);
    GetWorldTimerManager().SetTimer(
        TimerHandle_ResetChase,
        [this]() {
            bIsBeingChased = false;
            CurrentChaseIntensity = 0.0f;
        },
        ChaseMemoryTime,
        false
    );

    BP_OnChasedByMonster(ChaseIntensity);
}

void AHorrorGameCharacter::EnterDarknessZone(float DarknessIntensity)
{
    DarknessVolumeCount = FMath::Max(0, DarknessVolumeCount + 1);
    CurrentDarknessIntensity = FMath::Max(CurrentDarknessIntensity, DarknessIntensity);

    // Start darkness exposure timer
    if (DarknessVolumeCount == 1) // First darkness zone entered
    {
        DarknessExposureTime = 0.0f;
        StartDarknessExposure();
    }
}

void AHorrorGameCharacter::ExitDarknessZone(float DarknessIntensity)
{
    DarknessVolumeCount = FMath::Max(0, DarknessVolumeCount - 1);

    if (DarknessVolumeCount == 0)
    {
        CurrentDarknessIntensity = 0.0f;
        StopDarknessExposure();
    }
    else
    {
        // Recalculate darkness intensity if still in other darkness zones
        RecalculateDarknessIntensity();
    }
}

void AHorrorGameCharacter::StartDarknessExposure()
{
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_DarknessExposure,
        [this]()
        {
            DarknessExposureTime += 1.0f;

            // Progressive sanity drain in darkness
            if (SanityComponent && CurrentDarknessIntensity > 0.0f)
            {
                float DarknessDrain = CurrentDarknessIntensity * DarknessSanityDrainRate;
                SanityComponent->OnDarknessExposure(DarknessDrain);
            }
        },
        1.0f,  // Every second
        true   // Loop
    );
}

void AHorrorGameCharacter::StopDarknessExposure()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_DarknessExposure);
    DarknessExposureTime = 0.0f;
}

void AHorrorGameCharacter::PlayFalseSound()
{
    USoundBase* SoundToPlay = FalseHallucinationSound ? FalseHallucinationSound : AnxiousWhisperSound;
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, GetActorLocation());
        if (bDebugSanity)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayFalseSound: played hallucination sound"));
        }
    }
    else if (bDebugSanity)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayFalseSound: no sound assigned (FalseHallucinationSound / AnxiousWhisperSound)"));
    }
}

void AHorrorGameCharacter::TriggerWallMovementHallucination()
{
    // Minimal, general-purpose implementation: play a short camera shake and an unstable sound.
    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Warning, TEXT("TriggerWallMovementHallucination: triggered"));
    }

    if (UnstableSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, UnstableSound, GetActorLocation());
    }

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PanicCameraShake)
        {
            PC->PlayerCameraManager->StartCameraShake(PanicCameraShake, 0.6f);
        }
    }
}

void AHorrorGameCharacter::RecalculateDarknessIntensity()
{
    // If the character is inside explicit darkness volumes, those should have already set
    // CurrentDarknessIntensity via EnterDarknessZone/ExitDarknessZone. If not in a volume,
    // derive intensity from the CurrentLightLevel.
    if (DarknessVolumeCount > 0)
    {
        CurrentDarknessIntensity = FMath::Clamp(CurrentDarknessIntensity, 0.0f, 1.0f);
    }
    else
    {
        CurrentDarknessIntensity = FMath::Clamp(1.0f - CurrentLightLevel, 0.0f, 1.0f);
    }

    if (bDebugSanity)
    {
        UE_LOG(LogTemp, Verbose, TEXT("RecalculateDarknessIntensity -> %f (LightLevel=%f, Volumes=%d)"), CurrentDarknessIntensity, CurrentLightLevel, DarknessVolumeCount);
    }
}
