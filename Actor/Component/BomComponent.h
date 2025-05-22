#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BomComponent.generated.h"

// Forward declarations
class AGrenadeProjectile;
class UParticleSystem;
class UNiagaraSystem;
class AFireZone;
class UUserWidget;
class UPostProcessComponent;
class UMaterialInstanceDynamic;
class UCameraShakeBase;
class UCurveFloat;
class UTimelineComponent;
class UAnimMontage;
class USoundBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API UBomComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBomComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Các hàm xử lý liên quan đến bom
	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void HandleFlashExplosive();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void HandleMolotovCocktail();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void ThrowBomb(const FVector& TargetLocation, float ProjectileSpeed);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void ActivateAndThrowBomb(const FVector& TargetLocation, float ProjectileSpeed, bool bIsFlashBomb);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void SpawnAndThrowBomb(const FVector& Velocity);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	FVector CalculateTargetLocationForBomb();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	FVector GetSpawnLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	float ComputeInitialSpeed(float Distance, float Gravity, float AngleInRadians) const;

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	FVector ComputeLaunchVelocity(const FVector& SpawnLocation, const FVector& TargetLocation, float ProjectileSpeed, float AngleInRadians) const;

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void ExplodeFlash();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	bool IsPlayerLookingAtMe() const;

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void TriggerFlashWidget();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void TriggerFlashPostProcess();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void ApplyFlashEffectToScreen(APlayerController* PlayerController, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void UpdateFlashEffect(float Value);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void OnFlashTimelineFinished();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void FlashCurves();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void PlaySoundEffect(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void PlaySoundEffectForController(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void SpawnParticleEffect();

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void TriggerCameraShake(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void PlayFlashReactionMontage(APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void MolotovCocktail();

	//----------------------------------------------------------------
	// Các thuộc tính liên quan đến bom
	//----------------------------------------------------------------

	// Particle & Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MolotovCocktail")
	UParticleSystem* MolotovExplosive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flash")
	UParticleSystem* FlashExplosive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MolotovCocktail")
	UParticleSystem* IgniteEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MolotovCocktail")
	UNiagaraSystem* FireEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
	TSubclassOf<UUserWidget> FlashWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MolotovCocktail")
	TSubclassOf<AFireZone> FireZoneClass;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AGrenadeProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flash")
	UPostProcessComponent* FlashPostProcess;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flash")
	UMaterialInstanceDynamic* FlashMaterialInstance;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flash")
	//TSubclassOf<UCameraShakeBase> FlashCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flash")
	UTimelineComponent* FlashTimeline;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	UCurveFloat* FlashCurve;

	// Animation
	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	UAnimMontage* FlashReactionMontage;

	// Sound
	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	USoundBase* FlashSound;

	UPROPERTY(EditDefaultsOnly, Category = "MolotovCocktail")
	USoundBase* MolotovCocktailSound;

	// Các biến cấu hình
	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	float FlashMaxDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	float FlashViewAngleThreshold = 70.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	float FlashDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
	float FlashRadius = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bomb")
	float ActivationDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileSpeeds = 1000.0f;

	// Timer handle cho bomb activation
	UPROPERTY()
	FTimerHandle BombActivationTime;

	UPROPERTY()
	class AItem* CurrentBomb;
};
