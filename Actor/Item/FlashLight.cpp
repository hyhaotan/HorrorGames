// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Actor/Item/FlashLight.h"
#include "HorrorGame/Actor/Component/FlashLightComponent.h"
#include "Components/SpotLightComponent.h"

AFlashLight::AFlashLight()
{
    // Tạo component flashlight
    FlashLightComp = CreateDefaultSubobject<UFlashLightComponent>(TEXT("FlashLightComp"));
    FlashLightComp->SetupAttachment(ItemMesh);
}

void AFlashLight::UseItem()
{
    if (!FlashLightComp || !ItemMesh)
    {
        return;
    }

    // Lần đầu, attach spotlight vào socket "SpotLight" trên mesh
    if (!bFlashAttached)
    {
        FlashLightComp->SpotLight->AttachToComponent(
            ItemMesh,
            FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
            TEXT("SpotLight")
        );
        UE_LOG(LogTemp, Log, TEXT("Flashlight attached to item mesh."));
        bFlashAttached = true;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Flashlight already attached to item mesh."));
    }

    // Bật/Tắt spotlight
    FlashLightComp->ToggleFlashlight();
}
