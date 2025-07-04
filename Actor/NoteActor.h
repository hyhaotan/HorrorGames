// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Interface/Interact.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "NoteActor.generated.h"

class AElectronicLockActor;

UCLASS()
class HORRORGAME_API ANoteActor : public AInteractableActor, public IInteract
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// Sets default values for this actor's properties
	ANoteActor();

	UPROPERTY(EditAnywhere, Category = "Note")
	UTexture2D* NoteImage;

	UPROPERTY(EditAnywhere, Category = "Note")
	FText NoteText;

	virtual void Interact(AHorrorGameCharacter* Player) override;

	void HideNote();

	void ReturnToOriginal();

	UPROPERTY(EditAnywhere, Category = "Lock|Note")
	UTexture2D* NoteTexture;

	UPROPERTY(VisibleAnywhere, Category = "Lock")
	FString CodeString;

	UPROPERTY(VisibleAnywhere, Category = "Lock")
	TArray<FString> DigitStrings;

	/** Length of the code sequence */
	UPROPERTY(EditAnywhere, Category = "Lock")
	int32 CodeLength = 4;

	bool bNoteShown = false;

	UPROPERTY(BlueprintReadOnly,Category = "Lock")
	AElectronicLockActor* ElectronicLockActor;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Lock")
	TSubclassOf<AElectronicLockActor> ElectronicLockActorClass;

private:
	FTransform OriginalTransform;

	void TogglePlayerHUD(bool bVisible);

};