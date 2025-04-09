// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
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

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AHorrorGameCharacter : public ACharacter
{
	GENERATED_BODY()
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpotLightComponent* Light;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UPhysicsHandleComponent* PhysicsHandle;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SettingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HoldAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractGrabAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot1Action;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DropObjectAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ZoomObjectAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* UseItemAction;

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
	void IncreaseStamina(float Amount);

	UFUNCTION()
	void IncreaseStat(float& CurrentValue, float MaxValue, float Amount, const FString& StatName);
	//------------------------------------------------PROPERTY--------------------------------------------------------//
	//------------------------------------------------OTHER--------------------------------------------------------//
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

	UPROPERTY(EditAnywhere)
	TSubclassOf<UInventory> InventoryWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UInventorySlot> InventorySlotClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UDeathScreenWidget> DeathScreenWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PickupItemWidgetClass;

	UPROPERTY()
	UUserWidget* PickupItemWidget;

	UPROPERTY()
	AItem* EquippedItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PostProcess")
	class UPostProcessComponent* PostProcessComponent;

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

	//------------------------------------------------INT--------------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	int32 InteractLineTraceLength = 500;

	int32 CurrentHeldSlot;

	//------------------------------------------------ANIMATION--------------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* DeathMontage;
private:
	//------------------------------------------------FUNCTION--------------------------------------------------------//
	//Flashlight
	void ToggleFlashlight();
	void EnableFlashlight();
	void DisableFlashlight();

	void ToggleSettings();
	void GetSettingClass();

	void Interact();

	//Handle Everything Object
	void GrabObject();
	void StopGrabObject();
	void InteractWithGrabbedObject();
	void StoreCurrentHeldObject();
	AActor* GetHeldObject() const;
	void HandleAttachInteract(int32 Index);
	void RetrieveObject(int32 Index);
	void RetrieveObject1();
	void RetrieveObject2();
	void RetrieveObject3();
	void ToggleObject1();
	void ToggleObject2();
	void ToggleObject3();
	void DropObject();
	void HandleZoom(const FInputActionValue& Value);

	void HandleInventoryWidget();

	void Ticks(float DeltaTime);

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	//Handle Stamina and Sprint
	void HandleStaminaSprint(float DeltaTime);
	void EnableStaminaGain();
	void DepletedAllStamina();
	void Sprint();
	void UnSprint();

	// Handle Crouch
	void ToggleCrouch();

	UFUNCTION()
	void HandleDeath();

	void UseEquippedItem();

	void UpdatePickupWidget();

	bool PerformInteractionLineTrace(FHitResult& OutHitResult) const;

	//------------------------------------------------BOOLEAN--------------------------------------------------------//
	UPROPERTY(EditInstanceOnly, Category = "Crouch")
	bool bIsCrouching;	 
	
	UPROPERTY(EditDefaultsOnly, Category = "Hold Item")
	bool bIsHoldItem;
	//------------------------------------------------INTEGER--------------------------------------------------------//


	//------------------------------------------------FLOAT--------------------------------------------------------//
	
	//------------------------------------------------TIMER HANDLE--------------------------------------------------------//
	FTimerHandle StaminaRechargeTimerHandle;

	//------------------------------------------------OTHER--------------------------------------------------------//
	UMenuSettingWidget* MenuSettingWidget;

	AItem* ItemRef;
};

