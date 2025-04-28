// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Controller/GazeLockPlayerController.h"
#include "Kismet/KismetMathLibrary.h"

AGazeLockPlayerController::AGazeLockPlayerController()
{
    // Bật tick để Tick() được gọi mỗi frame
    PrimaryActorTick.bCanEverTick = true;
}

void AGazeLockPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsGazeLocked && GazeTarget.IsValid())
    {
        // Lấy vị trí & hướng camera hiện tại
        FVector CamLoc;
        FRotator CamRot;
        GetPlayerViewPoint(CamLoc, CamRot);

        // Tính rotation mong muốn nhìn thẳng vào target
        FVector ToTarget = (GazeTarget->GetActorLocation() - CamLoc).GetSafeNormal();
        FRotator DesiredRot = ToTarget.Rotation();

        // Lerp dần hướng nhìn của controller
        FRotator NewRot = FMath::RInterpTo(CamRot, DesiredRot, DeltaSeconds, GazeRotationSpeed);
        SetControlRotation(NewRot);

        // Đồng thời xoay nhân vật theo yaw mới
        if (APawn* MyPawn = GetPawn())
        {
            FRotator PawnRot = MyPawn->GetActorRotation();
            PawnRot.Yaw = NewRot.Yaw;
            MyPawn->SetActorRotation(PawnRot);
        }
    }
}

void AGazeLockPlayerController::ActivateGazeLock(AActor* InTarget, float InSpeed)
{
    if (!InTarget) return;
    GazeTarget = InTarget;
    GazeRotationSpeed = InSpeed;
    bIsGazeLocked = true;
}

void AGazeLockPlayerController::DeactivateGazeLock()
{
    bIsGazeLocked = false;
    GazeTarget = nullptr;
}