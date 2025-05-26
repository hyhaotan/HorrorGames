// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Interface/Interact.h"
#include "NoteActor.generated.h"

class UItemWidget;
class UWidgetComponent;
class USphereComponent;

UCLASS()
class HORRORGAME_API ANoteActor : public AActor, public IInteract
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// Sets default values for this actor's properties
	ANoteActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Note")
	UTexture2D* NoteImage;

	UPROPERTY(EditAnywhere, Category = "Note")
	FText NoteText;

	virtual void Interact(AHorrorGameCharacter* Player) override;

	void HideNote();

	void ReturnToOriginal();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Widget")
	UWidgetComponent* ItemWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box Item")
	USphereComponent* SphereComponent;

	UPROPERTY()
	UItemWidget* PickupWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
	TSubclassOf<UItemWidget> PickupWidgetClass;

private:
	FTransform OriginalTransform;
};