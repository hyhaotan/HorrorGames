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
class UCrossHairWidget;
class UKnockOutWidget;
class UNoteWidget;
class UKeyNotificationWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const TArray<AActor*>&, NewInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemToggled, int32, SlotIndex);

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	PS_Idle   UMETA(DisplayName = "Idle"),
	PS_Run    UMETA(DisplayName = "Run"),
	PS_Death  UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class EHallucinationType : uint8
{
	FakeMonster,
	FalseSound,
	WallMovement,
	ShadowPeople,
	FakeExit
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

	virtual void PostInitializeComponents() override;

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
	void IncreaseStat(float& CurrentValue, float MaxValue, float Amount, const FString& StatName);

	AActor* GetHeldObject() const;

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

	void SetCurrentNoteActor(ANoteActor* Note) { CurrentNote = Note; }

	void DropInventoryItem(bool bFromBag, int32 Index);

	void ClearCurrentInteractItem(AActor* ItemToClear)
	{
		if (Actors == ItemToClear)
			Actors = nullptr;
	}

	void SetInventoryVisible(bool bVisible);

	void TheChacterDeath();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AInteractableActor* Target);
	bool ServerInteract_Validate(AInteractableActor* Target);
	void ServerInteract_Implementation(AInteractableActor* Target);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickupItem(AItem* Item);
	bool ServerPickupItem_Validate(AItem* Item);
	void ServerPickupItem_Implementation(AItem* Item);

	void StoreCurrentHeldObject();

	// Multiplayer jumpscare functions
	UFUNCTION(Client, Reliable)
	void ClientStartJumpScare(class ANPC* JumpscareNPC, float BlendTime, float Duration);

	UFUNCTION(Client, Reliable)
	void ClientEndJumpScare(float BlendTime);

	UFUNCTION(Server, Reliable)
	void ServerStartKnockDown();

	UFUNCTION(Server, Reliable)
	void ServerStopKnockDown();

	// Knockdown functions (updated for multiplayer)
	UFUNCTION(BlueprintCallable)
	void StartKnockDown();

	UFUNCTION(BlueprintCallable)
	void StopKnockDown();

	UFUNCTION()
	bool IsKnockedDown() const { return bIsKnockedDown; };

	UFUNCTION()
	void OnRep_IsKnockedDown();

	UFUNCTION(BlueprintImplementableEvent, Category = "Headbob")
	void OnHeadbobStateChanged(EHeadbobState NewState, EHeadbobState OldState);

	// Returns sanity as a normalized value [0..1]
	UFUNCTION(BlueprintCallable, Category = "Sanity")
	float GetSanityLevel() const;

	// Returns computed fear level [0..1]
	UFUNCTION(BlueprintCallable, Category = "Sanity")
	float GetFearLevel() const;

	// Called when sanity crosses certain thresholds
	UFUNCTION()
	void OnSanityThresholdReached(float ThresholdPercent);

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void OnPanicAttack();

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void OnHallucination();

	// Notify character that a chase started (server or AI will call)
	UFUNCTION(BlueprintCallable, Category = "AI")
	void NotifyChasedByMonster(float ChaseIntensity);

	// Darkness zone enter/exit
	UFUNCTION(BlueprintCallable, Category = "Environment")
	void EnterDarknessZone(float DarknessIntensity);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void ExitDarknessZone(float DarknessIntensity);

	// State queries
	UFUNCTION(BlueprintPure, Category = "Sanity")
	bool IsBeingChased() const;

	UFUNCTION(BlueprintPure, Category = "Sanity")
	bool IsInDarkness() const;

	UFUNCTION(BlueprintPure, Category = "Sanity")
	bool IsInThreatProximity() const;

	/* ----------------------- BP Events / Hooks ----------------------- */
	UFUNCTION(BlueprintImplementableEvent, Category = "Sanity")
	void BP_OnSanityThresholdReached(float ThresholdPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sanity")
	void BP_OnPanicAttack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Sanity")
	void BP_OnHallucination();

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void BP_OnChasedByMonster(float ChaseIntensity);
	//------------------------------------------------PROPERTY--------------------------------------------------------//
	//------------------------------------------------OTHER--------------------------------------------------------//
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemToggled OnItemToggled;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY()
	AMonsterJump* GrabbingMonster;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="SettingWidget")
	TSubclassOf<UMenuSettingWidget> MenuSettingWidgetClass;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Inventory, Replicated, BlueprintReadOnly, Category = "Inventory")
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

	UPROPERTY(Replicated)
	AItem* EquippedItem;

	UPROPERTY(Replicated)
	AActor* EquippedActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PostProcess")
	class UPostProcessComponent* PostProcessComponent;

	/** PostProcess volume để blur/ghost */
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	UPostProcessComponent* PPComponent;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UKnockOutWidget> KnockOutWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TMap<EPlayerState, UAnimMontage*> AnimMontages;

	EPlayerState PlayerState = EPlayerState::PS_Death;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> NoteWidgetClass;

	//Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USprintComponent* SprintComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UHeadbobComponent* HeadbobComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USanityComponent* SanityComponent;

	UPROPERTY()
	class UFearComponent* FearComponent;
	//------------------------------------------------BOOLEAN--------------------------------------------------------//

	UPROPERTY(EditAnywhere, Replicated,BlueprintReadOnly, Category = "Flashlight")
	bool bIsFlashlightEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GrabbingObject")
	bool isGrabbingObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toggle Object 1")
	bool bIsToggleObject1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toggle Object 2")
	bool bIsToggleObject2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toggle Object 3")
	bool bIsToggleObject3;

	bool bIsPlayingPanicShake = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity")
	bool bIsGrabbed = false;

	bool bIsBagOpen = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsKnockedDown, BlueprintReadOnly)
	bool bIsKnockedDown = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Hold Item")
	bool bIsHoldingItem = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity")
	bool bIsBeingChased = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugSanity = false;

	//------------------------------------------------VECTOR--------------------------------------------------------//	  
	
	//------------------------------------------------FLOAT--------------------------------------------------------//
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float CurrentGrabDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float MinGrabDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float MaxGrabDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
	float Sanity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
	float MaxSanity = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knocked")
	float KnockedDownProgress = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knocked")
	float AmountKnockedDownProgress = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockdown")
	float KnockDownDuration = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity")
	float LastChaseTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity")
	float ChaseMemoryTime = 6.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity")
	float CurrentChaseIntensity = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
	float DarknessExposureTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
	float CurrentDarknessIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float DarknessSanityDrainRate = 0.5f; // sanity points per second * intensity

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float CurrentLightLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float DarknessThreshold = 0.3f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Threat")
	float ThreatProximityAmount = 0.0f; // normalized

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
	float ThreatProximityDistance = 600.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Threat")
	float NearestMonsterDistance = FLT_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
	float MaxFearDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PanicDuration = 3.0f;

	//------------------------------------------------INT--------------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	int32 InteractLineTraceLength = 500;

	int32 CurrentHeldSlot;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MainInventoryCapacity = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 EquippedIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
	int32 DarknessVolumeCount = 0;

	//------------------------------------------------ANIMATION--------------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockdown")
	UAnimMontage* KnockDownMontage;

	//------------------------------------------------OTHER--------------------------------------------------------//
	/* ----------------------- Audio / Effects ----------------------- */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* LowSanityAlarmSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* NervousBreathingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* AnxiousWhisperSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* UnstableSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* CriticalPanicSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* PanicAttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* FalseHallucinationSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<UCameraShakeBase> PanicCameraShake;

	/* ----------------------- Hallucinations ----------------------- */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hallucination")
	TArray<EHallucinationType> HallucinationTypes;
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
	void DropAllInventory();
	void PerformDrop(AActor* Actor, const FVector& DropLocation);
	FVector ComputeDropLocation(float Distance = 200.f) const;
	void HandleZoom(const FInputActionValue& Value);

	//-----------------------------------------------------------------------------//
	// INVENTORY
	//-----------------------------------------------------------------------------//

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
	//-----------------------------------------------------------------------------//
	// SANITY & HEADBOB
	//-----------------------------------------------------------------------------//

	//-----------------------------------------------------------------------------//
	// INPUT CALLBACKS & TIMELINE
	//-----------------------------------------------------------------------------//
	UFUNCTION()
	void OnEscape(const FInputActionValue& Value, FKey PressedKey);

	void Ticks(float DeltaTime);

	//-----------------------------------------------------------------------------//
	// OtHER FUNCTIONS
	//-----------------------------------------------------------------------------//
		// Helper methods
	float BaseSanityFear() const;
	float ChaseFear()       const;
	float DarknessFear()    const;

	//-----------------------------------------------------------------------------//
	// PROPERTIES
	//-----------------------------------------------------------------------------//

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

	FTimerHandle KnockDownTimerHandle;
	FTimerHandle JumpscareTimerHandle;
	FTimerHandle TimerHandle_ResetChase;
	FTimerHandle TimerHandle_DarknessExposure;

	UPROPERTY()
	AActor* OriginalViewTarget;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnJumpscareCameraComplete();

	UFUNCTION()
	void HandleHeadbobStateChanged(EHeadbobState NewState, EHeadbobState OldState);
	void EnableHeadbob(bool bEnable);
	void SetCustomHeadbob(TSubclassOf<UCameraShakeBase> CustomShake, float Intensity);

	UFUNCTION()
	void OnRep_Inventory();

	void TriggerRandomHallucination();

	void PlayFalseSound();
	void TriggerWallMovementHallucination();

	void RecalculateDarknessIntensity();

	void StartDarknessExposure();
	void StopDarknessExposure();
};

