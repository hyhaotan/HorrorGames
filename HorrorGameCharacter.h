// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGameCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMeshComponent;
class UMenuSettingWidget;
class UInventory;
class UInventorySlot;
class AItem;
struct FInputActionValue;
class AMonsterJump;
class USanityWidget;
class UTimelineComponent;
class UCurveFloat;
class ANoteActor;
class UInventoryBagWidget;
class UItemInfoWidget;
class UQuantitySelectionWidget;
class UCrossHairWidget;
class UKnockOutWidget;
class UNoteWidget;
class UKeyNotificationWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const TArray<AActor*>&, NewInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemQuantityChanged, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemToggled, int32, SlotIndex);

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	PS_Idle   UMETA(DisplayName = "Idle"),
	PS_Run    UMETA(DisplayName = "Run"),
	PS_Death  UMETA(DisplayName = "Death")
};

UCLASS(config=Game)
class AHorrorGameCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;          

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;           

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* ThirdPersonSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UPhysicsHandleComponent* PhysicsHandle;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* ClickAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* SettingAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* HoldAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* InteractGrabAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* Slot1Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* Slot2Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* Slot3Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* DropObjectAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* ZoomObjectAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* UseItemAction;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* EscapeAAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* EscapeSAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* EscapeWAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* EscapeDAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* UpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* DownAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* LeftAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* RightAction; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true")) UInputAction* TabAction;

public:
	AHorrorGameCharacter();
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

protected:
	//------------------------------------------------FUNCTION--------------------------------------------------------//
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
protected:
	//------------------------------------------------FUNCTION--------------------------------------------------------//
	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//------------------------------------------------OTHER--------------------------------------------------------//
public:

	//------------------------------------------------FUNCTION--------------------------------------------------------//
	UFUNCTION()
	float GetHealth()
	{
		return Health;
	};

	UFUNCTION()
	void IncreaseHealth(float Amount);

	UFUNCTION()
	void IncreaseStat(float& CurrentValue, float MaxValue, float Amount, const FString& StatName);

	AActor* GetHeldObject() const;

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void EnableFirstPerson();
	void EnableThirdPerson();

	UNoteWidget* ShowNoteUI(UTexture2D* NoteImage, const FText& NoteText);

	void CloseNoteUI();

	void SetCurrentInteractItem(AActor* NewItem) { Actors = NewItem; }

	void SetGrabbingMonster(AMonsterJump* Monster) { GrabbingMonster = Monster; }
	void ClearGrabbingMonster()
	{
		GrabbingMonster = nullptr;
		bIsPlayingPanicShake = false;
	}

	void RecoverSanity(float Delta);

	void PauseSanityDrain();
	void ResumeSanityDrain();

	void SetCurrentNoteActor(ANoteActor* Note) { CurrentNote = Note; }

	UFUNCTION()
	void SwapInventoryItems(bool SourceIsBag, int32 SourceIndex, bool TargetIsBag, int32 TargetIndex);

	void DropInventoryItem(bool bFromBag, int32 Index);

	void ClearCurrentInteractItem(AActor* ItemToClear)
	{
		if (Actors == ItemToClear)
			Actors = nullptr;
	}

	void SetInventoryVisible(bool bVisible);

	void TheChacterDeath();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteractWithSwitch(class ALightSwitchActor* SwitchActor);

	//------------------------------------------------PROPERTY--------------------------------------------------------//
	//------------------------------------------------OTHER--------------------------------------------------------//
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemToggled OnItemToggled;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnItemQuantityChanged OnItemQuantityChanged;

	UPROPERTY()
	AMonsterJump* GrabbingMonster;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="SettingWidget")
	TSubclassOf<UMenuSettingWidget> MenuSettingWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<AActor*> Inventory;

	UPROPERTY()
	UInventory* InventoryWidget;
	
	UPROPERTY()
	UInventorySlot* InventorySlot;

	/** Blueprint class của widget thông báo chìa khóa */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UKeyNotificationWidget> KeyNotificationWidgetClass;

	/** Instance widget để gọi show/update */
	UPROPERTY()
	UKeyNotificationWidget* KeyNotificationWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UInventory> InventoryWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UInventorySlot> InventorySlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UDeathScreenWidget> DeathScreenWidgetClass;

	UPROPERTY()
	AItem* EquippedItem;

	UPROPERTY()
	AActor* EquippedActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PostProcess")
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY(EditDefaultsOnly, Category = "HeadBob")
	TSubclassOf<UCameraShakeBase> WalkCameraShakeClass;

	/** Camera shake khi chạy */
	UPROPERTY(EditDefaultsOnly, Category = "HeadBob")
	TSubclassOf<UCameraShakeBase> SprintCameraShakeClass;

	/** Camera shake khi đứng yên hoặc rơi */
	UPROPERTY(EditDefaultsOnly, Category = "HeadBob")
	TSubclassOf<UCameraShakeBase> IdleCameraShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USanityWidget> SanityWidgetClass;

	USanityWidget* SanityWidget;

	/** Camera shake khi sanity thấp */
	UPROPERTY(EditAnywhere, Category = "Effects")
	TSubclassOf<UCameraShakeBase> LowSanityShake;

	/** PostProcess volume để blur/ghost */
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	UPostProcessComponent* PPComponent;

	/** Timeline for drain */
	UPROPERTY(EditAnywhere, Category = "Sanity")
	UCurveFloat* SanityDrainCurve;

	UPROPERTY()
	UTimelineComponent* SanityTimeline;

	UPROPERTY()
	TArray<AActor*> InventoryBag;

	UPROPERTY()
	UInventoryBagWidget* InventoryBagWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UInventoryBagWidget> InventoryBagWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UKnockOutWidget> KnockOutWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TMap<EPlayerState, UAnimMontage*> AnimMontages;

	EPlayerState PlayerState = EPlayerState::PS_Death;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> NoteWidgetClass;
	//------------------------------------------------BOOLEAN--------------------------------------------------------//

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
	bool bIsFlashlightEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GrabbingObject")
	bool isGrabbingObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toggle Object 1")
	bool bIsToggleObject1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toggle Object 2")
	bool bIsToggleObject2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toggle Object 3")
	bool bIsToggleObject3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	bool CanStaminaRecharge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	bool bIsSprint;

	bool bIsPlayingPanicShake = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity")
	bool bIsGrabbed = false;

	bool bIsBagOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knocked")
	bool bIsKnockedDown = false;
	//------------------------------------------------VECTOR--------------------------------------------------------//	  
	
	//------------------------------------------------FLOAT--------------------------------------------------------//
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float CurrentGrabDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float MinGrabDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float MaxGrabDistance = 500.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Heath", meta = (EditCondition = "Health >= 100"))
	float Health = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float DelayForStaminaRecharge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaSpringUsageRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRechargeRate;

	UPROPERTY(EditDefaultsOnly, Category = "HeadBob")
	float SprintSpeedThreshold = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
	float Sanity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
	float MaxSanity = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knocked")
	float KnockedDownProgress = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knocked")
	float AmountKnockedDownProgress = 0.05f;

	//------------------------------------------------INT--------------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	int32 InteractLineTraceLength = 500;

	int32 CurrentHeldSlot;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MainInventoryCapacity = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 BagCapacity = 10;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 EquippedIndex = INDEX_NONE;

	//------------------------------------------------ANIMATION--------------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* DeathMontage;

private:
	//-----------------------------------------------------------------------------//
	// SETTINGS & UI
	//-----------------------------------------------------------------------------//
	void ToggleSettings();
	void SetupWidgets();
	void HandleInventoryWidget();
	void HandleDeath();
	void UseEquippedItem();

	//-----------------------------------------------------------------------------//
	// INTERACTION
	//-----------------------------------------------------------------------------//
	void Interact();
	bool PerformInteractionLineTrace(FHitResult& OutHitResult) const;

	//-----------------------------------------------------------------------------//
	// GRAB & OBJECT MANAGEMENT
	//-----------------------------------------------------------------------------//
	void GrabObject();
	void StopGrabObject();
	void InteractWithGrabbedObject();
	void StoreCurrentHeldObject();

	void HandleAttachInteract(int32 Index);
	void RetrieveObject(int32 Index);
	void RetrieveObject1() { RetrieveObject(0); };
	void RetrieveObject2() { RetrieveObject(1); };
	void RetrieveObject3() { RetrieveObject(2); };
	void ToggleObject(int32 Index);
	UFUNCTION() void ToggleObject1() { ToggleObject(0); }
	UFUNCTION() void ToggleObject2() { ToggleObject(1); }
	UFUNCTION() void ToggleObject3() { ToggleObject(2); }
	void DropObject();
	void PerformDrop(AActor* Actor, const FVector& DropLocation);
	FVector ComputeDropLocation(float Distance = 200.f) const;
	void HandleZoom(const FInputActionValue& Value);

	//-----------------------------------------------------------------------------//
	// INVENTORY
	//-----------------------------------------------------------------------------//
	void ToggleInventoryBag();
	void HideInventoryBag();
	void ShowInventoryBag();

	bool TryStackIntoExisting(TArray<AActor*>& Container, AItem* NewItem);
	int32 CountValidSlots(const TArray<AActor*>& Container) const;
	void HandlePickup(AItem* NewItem, TArray<AActor*>& Container, UUserWidget* InventoryUI, bool bCanGrow);
	void RefreshUI(UUserWidget* InventoryUI, const TArray<AActor*>& Container);
	void RemoveWidgetsOfClasses(std::initializer_list<TSubclassOf<UUserWidget>> WidgetClasses);

	//-----------------------------------------------------------------------------//
	// CROUCH, SPRINT & STAMINA
	//-----------------------------------------------------------------------------//
	void ToggleCrouch();
	void Sprint();
	void UnSprint();
	void HandleStaminaSprint(float DeltaTime);
	void EnableStaminaGain();
	void DepletedAllStamina();

	//-----------------------------------------------------------------------------//
	// SANITY & HEADBOB
	//-----------------------------------------------------------------------------//
	void InitializeHeadbob();
	void SetupSanityWidget();
	void SetupSanityTimeline();

	//-----------------------------------------------------------------------------//
	// INPUT CALLBACKS & TIMELINE
	//-----------------------------------------------------------------------------//
	UFUNCTION()
	void OnEscape(const FInputActionValue& Value, FKey PressedKey);

	UFUNCTION()
	void HandleDrainProgress(float Value);

	void Ticks(float DeltaTime);

	//-----------------------------------------------------------------------------//
	// OtHER FUNCTIONS
	//-----------------------------------------------------------------------------//

	void StartKnockDown();
	void StopKnockDown();

	//-----------------------------------------------------------------------------//
	// PROPERTIES
	//-----------------------------------------------------------------------------//
	UPROPERTY(EditInstanceOnly, Category = "Crouch")
	bool bIsCrouching;

	FTimerHandle StaminaRechargeTimerHandle;

	UPROPERTY()
	class UMenuSettingWidget* MenuSettingWidget;

	UPROPERTY()
	class UNoteWidget* NoteWidgetInstance = nullptr;
	UPROPERTY()
	class ANoteActor* CurrentNote = nullptr;

	UPROPERTY()
	UKnockOutWidget* KnockOutWidgetInstance = nullptr;

	UPROPERTY(EditAnywhere, Category = "CrossHair")
	UTexture2D* CrossHairIcon;

	UPROPERTY()
	class AItem* ItemRef = nullptr;
	UPROPERTY()
	class AItem* HighlightedItem;
	UPROPERTY()
	AActor* Actors = nullptr;

	IInteract* CurrentInteract = nullptr;

};

