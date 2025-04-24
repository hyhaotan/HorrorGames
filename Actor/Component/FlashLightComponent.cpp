#include "FlashLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "HorrorGame/Actor/EyeMonster.h"

UFlashLightComponent::UFlashLightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsOn = false;

    // Tạo con SpotLight và mặc định tắt
    SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashSpotLight"));
    SpotLight->SetupAttachment(this);
    SpotLight->SetVisibility(false);
    SpotLight->bUseInverseSquaredFalloff = false; // nếu muốn linear attenuation
    SpotLight->Intensity = 5000.f;
    SpotLight->InnerConeAngle = 10.f;
    SpotLight->OuterConeAngle = 20.f;
}

void UFlashLightComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UFlashLightComponent::ToggleFlashlight()
{
    bIsOn = !bIsOn;
    SpotLight->SetVisibility(bIsOn);
    SetComponentTickEnabled(bIsOn);
}

void UFlashLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsOn)
    {
        HandleFlashLight();
    }
}

void UFlashLightComponent::HandleFlashLight()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Loc;
    FRotator Rot;
    Owner->GetActorEyesViewPoint(Loc, Rot);

    SpotLight->SetWorldLocation(Loc);
    SpotLight->SetWorldRotation(Rot);

    // Trace
    FVector End = Loc + Rot.Vector() * TraceDistance;
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    AEyeMonster* HitMonster = nullptr;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Loc, End, ECC_Visibility, Params))
    {
        HitMonster = Cast<AEyeMonster>(Hit.GetActor());
        if (HitMonster)
        {
            FVector Dir = (HitMonster->GetActorLocation() - Loc).GetSafeNormal();
            float Dot = FVector::DotProduct(Rot.Vector(), Dir);
            if (Dot < LightDotThreshold)
            {
                HitMonster = nullptr;
            }
        }
    }

    // Nếu có hit và góc thoả, set illuminated = true
    if (HitMonster)
    {
        HitMonster->SetIlluminated(true);
        LastIlluminatedMonster = HitMonster;
    }
    else if (LastIlluminatedMonster.IsValid())
    {
        // Ngược lại, tắt trạng thái của monster cũ
        LastIlluminatedMonster->SetIlluminated(false);
        LastIlluminatedMonster = nullptr;
    }
}
