#include "HorrorGame/Actor/FireZone.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AFireZone::AFireZone()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;

    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->SetCollisionResponseToAllChannels(ECR_Overlap);

    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AFireZone::OnOverlapBegin);
    CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AFireZone::OnOverlapEnd);

    BurnDamage = 5.f;
    BurnDamageInterval = 1.f;
}

void AFireZone::BeginPlay()
{
    Super::BeginPlay();
}

void AFireZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (AHorrorGameCharacter* Character = Cast<AHorrorGameCharacter>(OtherActor))
    {
        // Bắt đầu gây sát thương nếu chưa có
        StartBurnDamage(Character);
    }
}

void AFireZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AHorrorGameCharacter* Character = Cast<AHorrorGameCharacter>(OtherActor))
    {
        // Spawn hiệu ứng cháy lên người chơi
        if (BurningEffect && !ActiveBurningEffects.Contains(Character))
        {
            UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
                BurningEffect,
                Character->GetMesh(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::SnapToTargetIncludingScale,
                true
            );

            if (NiagaraComp)
            {
                ActiveBurningEffects.Add(Character, NiagaraComp);

                // Sau 3 giây thì xóa hiệu ứng và dừng sát thương
                FTimerHandle BurnRemovalTimer;
                GetWorld()->GetTimerManager().SetTimer(BurnRemovalTimer, FTimerDelegate::CreateLambda([this, Character, NiagaraComp]()
                    {
                        RemoveBurningEffect(Character, NiagaraComp);
                    }), 3.0f, false);
            }
        }
    }
}

void AFireZone::StartBurnDamage(AHorrorGameCharacter* Character)
{
    if (!Character || DamageTimerMap.Contains(Character))
    {
        return;
    }

    TSharedPtr<int32> TickCount = MakeShared<int32>(0);
    FTimerHandle LocalDamageTimerHandle;

    FTimerDelegate DamageDelegate = FTimerDelegate::CreateLambda(
        [this, Character, TickCount]() mutable
        {
            if (!IsValid(Character))
            {
                DamageTimerMap.Remove(Character);
                return;
            }

            UGameplayStatics::ApplyDamage(Character, BurnDamage, nullptr, this, UDamageType::StaticClass());
            (*TickCount)++;
        }
    );

    GetWorld()->GetTimerManager().SetTimer(LocalDamageTimerHandle, DamageDelegate, BurnDamageInterval, true);
    DamageTimerMap.Add(Character, LocalDamageTimerHandle);
}

void AFireZone::RemoveBurningEffect(AHorrorGameCharacter* Character, UNiagaraComponent* NiagaraComp)
{
    if (NiagaraComp)
    {
        NiagaraComp->DestroyComponent();
    }

    ActiveBurningEffects.Remove(Character);

    if (DamageTimerMap.Contains(Character))
    {
        GetWorld()->GetTimerManager().ClearTimer(DamageTimerMap[Character]);
        DamageTimerMap.Remove(Character);
    }
}

void AFireZone::ApplyBurnDamage(AActor* AffectedActor)
{
    if (AffectedActor)
    {
        UGameplayStatics::ApplyDamage(AffectedActor, BurnDamage, nullptr, this, UDamageType::StaticClass());
    }
}
