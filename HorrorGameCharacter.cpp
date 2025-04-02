// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorGameCharacter.h"
#include "HorrorGame/Actor/Door.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGame/Widget/Settings/MenuSettingWidget.h"
#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/Inventory/Inventory.h"
#include "HorrorGame/Widget/Inventory/InventorySlot.h"
#include "HorrorGame/Widget/Inventory/InventoryItem.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/Widget/Settings/DeathScreenWidget.h"

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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AHorrorGameCharacter

AHorrorGameCharacter::AHorrorGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (attach to socket "head" của Mesh)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), TEXT("head"));
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->CameraLagSpeed = 9.0f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 10.0f, 0.0f));

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = true;

	// Create flashlight (SpotLight)
	Light = CreateDefaultSubobject<USpotLightComponent>(TEXT("Light"));
	Light->SetupAttachment(FollowCamera);
	Light->Intensity = 10000.0f;
	Light->AttenuationRadius = 1000.0f;
	Light->OuterConeAngle = 25.0f;
	Light->InnerConeAngle = 18.0f;
	Light->SetVisibility(false);

	// Initialize physics variables
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	// Khởi tạo trạng thái flashlight và việc đang giữ vật
	bIsFlashlightEnabled = false;
	isGrabbingObject = false;

    // Configure character movement
    GetCharacterMovement()->MaxWalkSpeed = 200.f;

    //Set Stamina
    CurrentStamina = 1.f;
    MaxStamina = 1.f;
    StaminaSpringUsageRate = 0.1f;
    StaminaRechargeRate = 0.1f;

    CanStaminaRecharge = true;
    DelayForStaminaRecharge = 2.0f;

	//Set Crouch
	bIsCrouching = false;
}

void AHorrorGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetSettingClass();

    HandleInventoryWidget();
}

void AHorrorGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Ticks(DeltaTime);

	HandleStaminaSprint(DeltaTime);
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
		// Flashlight
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleFlashlight);
		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::Interact);
		// Settings
		EnhancedInputComponent->BindAction(SettingAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::ToggleSettings);

		// Object interactions: Grab, Retrieve (attach) and Drop,....
		EnhancedInputComponent->BindAction(HoldAction, ETriggerEvent::Triggered, this, &AHorrorGameCharacter::GrabObject);
		EnhancedInputComponent->BindAction(HoldAction, ETriggerEvent::Completed, this, &AHorrorGameCharacter::StopGrabObject);
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
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AHorrorGameCharacter::ToggleFlashlight()
{
	if (!bIsFlashlightEnabled)
	{
		EnableFlashlight();
	}
	else
	{
		DisableFlashlight();
	}
}

void AHorrorGameCharacter::EnableFlashlight()
{
	bIsFlashlightEnabled = true;
	Light->SetVisibility(true);
}

void AHorrorGameCharacter::DisableFlashlight()
{
	bIsFlashlightEnabled = false;
	Light->SetVisibility(false);
}

void AHorrorGameCharacter::ToggleSettings()
{
	if (MenuSettingWidget)
	{
		MenuSettingWidget->DisplayMenu();
	}
}

void AHorrorGameCharacter::GetSettingClass()
{
	if (MenuSettingWidgetClass)
	{
		MenuSettingWidget = CreateWidget<UMenuSettingWidget>(GetWorld(), MenuSettingWidgetClass);
		if (MenuSettingWidget)
		{
			MenuSettingWidget->AddToViewport();
			MenuSettingWidget->SetVisibility(ESlateVisibility::Collapsed);
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

void AHorrorGameCharacter::Interact()
{
	FHitResult HitResult;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * InteractLineTraceLength;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
	if (ADoor* Door = Cast<ADoor>(HitResult.GetActor()))
	{
		Door->Player = this;
		Door->Interact();
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
            if (ItemRef->ItemMesh)
            {
                // Kiểm tra nếu ItemData đã được khởi tạo
                if (ItemRef->ItemData)
                {
                    int32 WeightItem = ItemRef->ItemData->ItemQuantityData.Weight;
                    // hoặc: int32 WeightItem = ItemRef->ItemData->GetWeight();
                    ItemRef->SetItemWeight(WeightItem);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("ItemData is null!"));
                }

                // Giảm InterpolationSpeed để tạo cảm giác nặng hơn
                PhysicsHandle->InterpolationSpeed = 5.0f;

                // Đảm bảo simulate physics đang được bật cho item
                if (!ItemRef->ItemMesh->IsSimulatingPhysics())
                {
                    ItemRef->ItemMesh->SetSimulatePhysics(true);
                }

                // Tính khoảng cách ban đầu giữa camera và vị trí va chạm
                CurrentGrabDistance = (HitResult.Location - Start).Size();

                // Dùng Physics Handle để bắt giữ item
                PhysicsHandle->GrabComponentAtLocationWithRotation(
                    ItemRef->ItemMesh,
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
    if (isGrabbingObject && PhysicsHandle && PhysicsHandle->GrabbedComponent)
    {
        // Kiểm tra số lượng item hiện có trong inventory
        int32 ValidItemCount = 0;
        for (AActor* Actor : Inventory)
        {
            if (Actor != nullptr)
            {
                ValidItemCount++;
            }
        }
        if (ValidItemCount >= 3)
        {
            UE_LOG(LogTemp, Warning, TEXT("Inventory đầy, không thể nhặt thêm item."));
            // Giải phóng vật thể đang giữ nếu inventory đã đầy
            PhysicsHandle->ReleaseComponent();
            isGrabbingObject = false;
            return;
        }

        AActor* GrabbedActor = PhysicsHandle->GrabbedComponent->GetOwner();
        if (GrabbedActor)
        {
            // Thêm đối tượng vừa nhặt vào Inventory (tìm slot trống nếu có)
            int32 EmptyIndex = Inventory.IndexOfByPredicate([](AActor* Actor) { return Actor == nullptr; });
            if (EmptyIndex != INDEX_NONE)
            {
                Inventory[EmptyIndex] = GrabbedActor;
            }
            else
            {
                Inventory.Add(GrabbedActor);
            }

            // Nếu đối tượng là AItem, gọi hàm OnPickup để xử lý ẩn, tắt collision và physics
            if (AItem* Item = Cast<AItem>(GrabbedActor))
            {
                Item->OnPickup();
            }
            PhysicsHandle->ReleaseComponent();
            isGrabbingObject = false;
            // Reset ItemRef sau khi nhặt xong
            ItemRef = nullptr;
        }

        if (InventoryWidget)
        {
            InventoryWidget->UpdateInventory(Inventory);
        }
    }
}

void AHorrorGameCharacter::StoreCurrentHeldObject()
{
    if (AActor* HeldObject = GetHeldObject())
    {
        HeldObject->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        if (!Inventory.Contains(HeldObject))
        {
            Inventory.Add(HeldObject);
            UE_LOG(LogTemp, Warning, TEXT("Stored currently held object into Inventory."));
        }
        if (AItem* Item = Cast<AItem>(HeldObject))
        {
            Item->OnPickup();
        }
        else
        {
            HeldObject->SetActorHiddenInGame(true);
        }
    }
}

AActor* AHorrorGameCharacter::GetHeldObject() const
{
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    for (AActor* Actor : AttachedActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            if (Actor->GetRootComponent()->GetAttachParent() == GetMesh() &&
                Actor->GetRootComponent()->GetAttachSocketName() == FName("Object"))
            {
                return Actor;
            }
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
    AActor* RetrievedActor = Inventory[Index];
    if (!RetrievedActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("No object at Inventory index: %d"), Index);
        return;
    }

    // Nếu đối tượng là AItem, gọi hàm AttachToCharacter bên AItem
    if (AItem* Item = Cast<AItem>(RetrievedActor))
    {
        Item->AttachToCharacter(GetMesh(), FName("Object"));
    }
    else
    {
        // Với đối tượng không phải AItem, xử lý attach thủ công
        RetrievedActor->SetActorHiddenInGame(false);
        RetrievedActor->SetActorEnableCollision(false);
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(RetrievedActor->GetComponentByClass(UPrimitiveComponent::StaticClass())))
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            if (PrimComp->IsSimulatingPhysics())
            {
                PrimComp->SetSimulatePhysics(false);
            }
        }
        RetrievedActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Object"));
    }
    UE_LOG(LogTemp, Warning, TEXT("Attached object from Inventory slot %d to hand."), Index);
}

void AHorrorGameCharacter::RetrieveObject(int32 Index)
{
    StoreCurrentHeldObject();
    HandleAttachInteract(Index);
}

void AHorrorGameCharacter::RetrieveObject1()
{
    RetrieveObject(0);
}

void AHorrorGameCharacter::RetrieveObject2()
{
    RetrieveObject(1);
}

void AHorrorGameCharacter::RetrieveObject3()
{
    RetrieveObject(2);
}

void AHorrorGameCharacter::ToggleObject1()
{
    if (bIsToggleObject1)
    {
        StoreCurrentHeldObject();
        bIsToggleObject1 = false;
    }
    else
    {
        RetrieveObject1();
        bIsToggleObject1 = true;
        bIsToggleObject2 = false;
        bIsToggleObject3 = false;
    }
}

void AHorrorGameCharacter::ToggleObject2()
{
    if (bIsToggleObject2)
    {
        StoreCurrentHeldObject();
        bIsToggleObject2 = false;
    }
    else
    {
        RetrieveObject2();
        bIsToggleObject1 = false;
        bIsToggleObject2 = true;
        bIsToggleObject3 = false;
    }
}

void AHorrorGameCharacter::ToggleObject3()
{
    if (bIsToggleObject3)
    {
        StoreCurrentHeldObject();
        bIsToggleObject3 = false;
    }
    else
    {
        RetrieveObject3();
        bIsToggleObject1 = false;
        bIsToggleObject2 = false;
        bIsToggleObject3 = true;
    }
}

void AHorrorGameCharacter::DropObject()
{
    AActor* HeldObject = GetHeldObject();
    if (HeldObject)
    {
        HeldObject->SetActorEnableCollision(true);
        int32 DroppedIndex = Inventory.Find(HeldObject);
        if (DroppedIndex != INDEX_NONE)
        {
            HeldObject->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            if (FollowCamera)
            {
                FVector CameraLocation = FollowCamera->GetComponentLocation();
                FVector CameraForward = FollowCamera->GetForwardVector();
                float DropDistance = 200.0f;
                FVector DropLocation = CameraLocation + CameraForward * DropDistance;

                // Nếu là AItem thì gọi hàm PrepareForDrop để xử lý hiển thị, collision, physics
                if (AItem* Item = Cast<AItem>(HeldObject))
                {
                    Item->OnDrop(DropLocation);
                }
                else
                {
                    HeldObject->SetActorHiddenInGame(false);
                    HeldObject->SetActorLocation(DropLocation);
                    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldObject->GetComponentByClass(UPrimitiveComponent::StaticClass())))
                    {
                        PrimComp->SetSimulatePhysics(true);
                        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    }
                }
            }
            // Xóa đối tượng khỏi Inventory (slot trở thành nullptr)
            Inventory[DroppedIndex] = nullptr;

            if (InventoryWidget)
            {
                InventoryWidget->UpdateInventory(Inventory);
            }

            UE_LOG(LogTemp, Warning, TEXT("Dropped object from Inventory slot %d"), DroppedIndex);
        }
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
    if (bIsCrouching)
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
    // Nếu đang crouch, không cho phép sprint và giữ nguyên trạng thái crouch
    if (bIsCrouching)
    {
        UE_LOG(LogTemp, Warning, TEXT("Không thể chạy khi đang cúi người."));
        return;
    }

    // Chỉ cho phép sprint khi nhân vật đang di chuyển
    if (GetVelocity().SizeSquared() <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Không thể chạy khi đứng yên."));
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
    UE_LOG(LogTemp, Warning, TEXT("Chạy với tốc độ: %f"), SprintSpeed);

    // Ngăn việc hồi phục stamina trong lúc sprint
    CanStaminaRecharge = false;
    GetWorld()->GetTimerManager().ClearTimer(StaminaRechargeTimerHandle);
}



void AHorrorGameCharacter::UnSprint()
{
    if (bIsSprint)
    {
        UE_LOG(LogTemp, Warning, TEXT("We have stopped running."));
        bIsSprint = false;
        GetCharacterMovement()->MaxWalkSpeed = 200.f;

        //Start the timer to rechage stamina when the character has stopped running
        GetWorld()->GetTimerManager().SetTimer(StaminaRechargeTimerHandle, this, &AHorrorGameCharacter::EnableStaminaGain, DelayForStaminaRecharge, false);
    }
}

void AHorrorGameCharacter::ToggleCrouch()
{
	if (bIsCrouching)
	{
		UnCrouch();
        bIsCrouching = false;
	}
	else
	{
		Crouch();
        bIsCrouching = true;
	}
}

float AHorrorGameCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Gọi hàm TakeDamage của lớp cha để lấy sát thương thực tế
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Giảm máu theo sát thương nhận được
    Health -= ActualDamage;
    UE_LOG(LogTemp, Warning, TEXT("Player bị sát thương: %f, máu còn lại: %f"), ActualDamage, Health);

    // Nếu máu dưới hoặc bằng 0, thực hiện play animation death, sau đó hiển thị death screen
    if (Health <= 0)
    {
        // Phát hoạt ảnh chết (nếu có)
        if (DeathMontage)
        {
            PlayAnimMontage(DeathMontage, 1.0f);
        }
        UE_LOG(LogTemp, Error, TEXT("Máu <= 0, player chết. Đợi 3 giây rồi hiển thị death screen!"));

        // Dùng Timer để trì hoãn 3 giây trước khi hiển thị Death Screen
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, this, &AHorrorGameCharacter::HandleDeath, 3.0f, false);
    }

    return ActualDamage;
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
        // Vô hiệu hoá nhập liệu để người chơi không thể tương tác
        PC->SetCinematicMode(true, false, false, true, true);
        // Tạm dừng game
        PC->SetPause(true);
        // Hiển thị chuột khi chết
        PC->SetShowMouseCursor(true);
    }
}