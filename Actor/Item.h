#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Data/ItemData.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

class UParticleSystem;
class UItemBase;
class UPostProcessComponent;
class UMaterialInstanceDynamic;
class UCameraShakeBase;
class UCurveFloat;
class UUserWidget;
class UAnimMontage;
class USoundBase;
class UTimelineComponent;

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	General UMETA(DisplayName = "General"),
	Flash UMETA(DisplayName = "Flash"),
	HeathMedicine UMETA(DisplayName = "HeathMedicine"),
	StaminaMedicine UMETA(DisplayName = "StaminaMedicine"),
};

UCLASS()
class HORRORGAME_API AItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//-----------------------------------------------------COMPONENTS & PROPERTIES-----------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Widget")
	class UWidgetComponent* ItemWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box Item")
	class UBoxComponent* ItemCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Mesh")
	class UStaticMeshComponent* ItemMesh;

	// Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FDataTableRowHandle ItemRowHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	UItemBase* ItemData;

	// Item category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemCategory ItemCategory;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* GeneralExplosive;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* FlashExplosive;

	UPROPERTY(EditAnywhere, Category = "Flash | Widget")
	TSubclassOf<UUserWidget> FlashWidgetClass;

	UPROPERTY()
	UPostProcessComponent* FlashPostProcess;

	UPROPERTY()
	UMaterialInstanceDynamic* FlashMaterialInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	TSubclassOf<UCameraShakeBase> FlashCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	UParticleSystem* ExplosionEffect;

	UPROPERTY()
	UTimelineComponent* FlashTimeline;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	UCurveFloat* FlashCurve;

	//------------------------------------------------------ANIMATION-----------------------------------------------------//
	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	UAnimMontage* FlashReactionMontage;

	//------------------------------------------------------SOUND-----------------------------------------------------//
	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	USoundBase* FlashSound;

	//------------------------------------------------------VARIABLES-----------------------------------------------------//
	//------------------------------------------------------FLOAT-----------------------------------------------------//
	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	float FlashMaxDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	float FlashViewAngleThreshold = 70.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	float FlashDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
	float FlashRadius = 800.0f;
	//-----------------------------------------------------FUNCTIONS-----------------------------------------------------//
	UFUNCTION(BlueprintCallable, Category = "Item")
	void OnPickup();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void OnDrop(const FVector& DropLocation);

	UFUNCTION(BlueprintCallable, Category = "Item Weight")
	void SetItemWeight(int32 Weight);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleHealthMedicine();

	UFUNCTION()
	void HandleStaminaMedicine();

	UFUNCTION()
	void HandleGeneralExplosive();

	UFUNCTION()
	void ExplodeFlash();

	UFUNCTION()
	void ExplodeGeneral();

	void HandleUseItem();

	UFUNCTION()
	void HandleFlashExplosive();

	UFUNCTION()
	float ActivationTime(float Seconds);

	void ApplyFlashEffectToScreen(APlayerController* PlayerController, float Duration);

	void PlaySoundEffect(const FVector& Location);

	void PlaySoundEffectForController(APlayerController* PC);

	void SpawnParticleEffect();

	void TriggerCameraShake(APlayerController* PC);

	void PlayFlashReactionMontage(APawn* Pawn);

private:
	// Hàm khởi tạo dữ liệu cho item dựa vào DataTable
	void InitializeItemData();

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UMaterialInterface* FlashPostProcessMaterial;

	FTimerHandle BombActivationTime;

	bool IsPlayerLookingAtMe() const;
	void TriggerFlashWidget();
	void TriggerFlashPostProcess();

	// --- Hàm callback cho Timeline ---
	UFUNCTION()
	void UpdateFlashEffect(float Value);

	UFUNCTION()
	void OnFlashTimelineFinished();

	UFUNCTION()
	void FlashCurves();

};
