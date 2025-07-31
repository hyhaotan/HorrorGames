// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "LightActor.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;

UCLASS()
class HORRORGAME_API ALightActor : public AInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALightActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (AllowPrivateAccess))
	UStaticMeshComponent* LightMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (AllowPrivateAccess))
	USpotLightComponent* SpotLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (AllowPrivateAccess))
	bool bIsLightOn = false;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:


};
