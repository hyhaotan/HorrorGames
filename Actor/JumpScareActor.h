#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpScareActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumpScareCompleted);

UCLASS()
class HORRORGAME_API AJumpScareActor : public AActor
{
	GENERATED_BODY()

public:
	AJumpScareActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* StartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* EndLocation;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpScare")
	float JumpScareSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpScare")
	float FinishDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* JumpScareSound;

	// Event (Blueprint can bind)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnJumpScareCompleted OnJumpScareCompleted;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "JumpScare")
	void TriggerJumpScare();

	UFUNCTION(BlueprintCallable, Category = "JumpScare")
	void CompleteJumpScare();

	// Overlap
	UFUNCTION()
	void OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

private:
	// runtime state
	bool bIsJumpScareActive;
	bool bSoundPlayed;
	FVector MoveDirection;
};
