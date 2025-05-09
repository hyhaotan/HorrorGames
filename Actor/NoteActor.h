// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Interface/Interact.h"
#include "NoteActor.generated.h"

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

	virtual void Interact(AActor* Interactor) override;

	void HideNote();

	void ReturnToOriginal();

private:
	FTransform OriginalTransform;
};