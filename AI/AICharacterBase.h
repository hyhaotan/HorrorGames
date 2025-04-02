// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AICharacterBase.generated.h"

//class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API AAICharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacterBase();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//set value GetHealth and GetMaxHealth
	float GetHealth()
	{
		return Health;
	}
	float GetMaxHealth()
	{
		return MaxHealth;
	}

	void SetHealth(float const NewHealth);
	void AttackStart() const;
	void AttackEnd() const;

	float MaxHealth = 100.f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class UWidgetComponent* WidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = COLLISION, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* RightFistCollisionBox;

	UFUNCTION()
	void OnAttackOverlapBegin(
		UPrimitiveComponent* const OverlapComponent,
		AActor* const OtherActor,
		UPrimitiveComponent* const OtheComponent,
		int const OtherBodyIndex,
		bool const FromSweep,
		FHitResult const& SweepResult
	);

	UFUNCTION()
	void OnAttackOverlapEnd(
		UPrimitiveComponent* const OverlapComponent,
		AActor* const OtherActor,
		UPrimitiveComponent* const OtherComponent,
		int const OtherBodyIndex
	);

public:
	float Health;

	UPROPERTY(EditAnywhere, Category = "AI",BlueprintReadOnly)
	class UTextRenderComponent* AlertTextComponent;
};
