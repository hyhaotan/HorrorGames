// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GazeLockPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HORRORGAME_API AGazeLockPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    AGazeLockPlayerController();
    virtual void Tick(float DeltaSeconds) override;

    /** Kích hoạt gaze lock vào một target với tốc độ xoay */
    void ActivateGazeLock(AActor* InTarget, float InSpeed);

    /** Hủy gaze lock */
    void DeactivateGazeLock();

protected:
    /** Actor mà camera và nhân vật đang bị khóa vào */
    TWeakObjectPtr<AActor> GazeTarget;
    bool bIsGazeLocked = false;
    float GazeRotationSpeed = 5.f;
};
