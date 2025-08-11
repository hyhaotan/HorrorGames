#include "FearComponent.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "SanityComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Actor/MonsterJump.h"

UFearComponent::UFearComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UFearComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerChar = Cast<AHorrorGameCharacter>(GetOwner());
    if (OwnerChar)
    {
        SanityComp = OwnerChar->FindComponentByClass<USanityComponent>();
    }
}

float UFearComponent::GetFearLevel() const
{
    float Fear = 0.f;
    Fear += BaseSanityFear();
    Fear += ChaseFear();
    Fear += DarknessFear();
    Fear += ThreatProximityFear();

    // Áp curve nếu có
    if (FearCurve)
    {
        Fear = FearCurve->GetFloatValue(Fear);
    }

    return FMath::Clamp(Fear, 0.f, 1.f);
}

float UFearComponent::BaseSanityFear() const
{
    return SanityComp
        ? 1.f - SanityComp->GetSanityPercent()
        : 0.f;
}

float UFearComponent::ChaseFear() const
{
    return (OwnerChar && OwnerChar->IsBeingChased())
        ? ChaseFearAmount
        : 0.f;
}

float UFearComponent::DarknessFear() const
{
    return (OwnerChar && OwnerChar->IsInDarkness())
        ? DarknessFearAmount
        : 0.f;
}

float UFearComponent::ThreatProximityFear() const
{
    if (!OwnerChar || ThreatRadius <= 0.f)
        return 0.f;

    FVector Loc = OwnerChar->GetActorLocation();
    float BestNorm = 0.f;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ThreatRadius);

    if (GetWorld()->OverlapMultiByChannel(
        Overlaps, Loc, FQuat::Identity,
        ECC_Pawn, Sphere))
    {
        for (auto& Result : Overlaps)
        {
            AActor* Other = Result.GetActor();
            if (Other && Other != OwnerChar && Other->IsA(AMonsterJump::StaticClass()))
            {
                float Dist = FVector::Dist(Loc, Other->GetActorLocation());
                float Norm = 1.f - FMath::Clamp(Dist / ThreatRadius, 0.f, 1.f);
                BestNorm = FMath::Max(BestNorm, Norm);
            }
        }
    }

    return BestNorm * ThreatProximityScale;
}
