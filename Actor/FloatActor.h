// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatActor.generated.h"

UCLASS()
class HORRORGAME_API AFloatActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloatActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	UPROPERTY(EditAnywhere, Category = "Cube")
	class UStaticMeshComponent* CubeMesh;

	UPROPERTY(EditAnywhere, Category = "FloatSpeed")
	float FloatSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "CubeSpeed")
	float RotationSpeed = 20.f;
};
