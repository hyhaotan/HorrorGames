#include "LightSwitchActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ALightSwitchActor::ALightSwitchActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SetReplicates(true);
    SetReplicateMovement(true);

    LightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightMesh"));
    RootComponent = LightMesh;

    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    PointLight->SetupAttachment(LightMesh);
    PointLight->SetVisibility(false);

    SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
    SwitchMesh->SetupAttachment(LightMesh);
}

void ALightSwitchActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALightSwitchActor, bIsOn);
}

void ALightSwitchActor::Interact(AHorrorGameCharacter* Interactor)
{
    if (!Interactor) return;

    Interactor->SetCurrentInteractItem(this);

    Interactor->ServerInteract(this);
}

void ALightSwitchActor::MulticastToggleLightSwitch_Implementation()
{
    DoToggle();
}

void ALightSwitchActor::OnRep_IsOn()
{
    PointLight->SetVisibility(bIsOn);
    SwitchMesh->SetRelativeRotation(bIsOn
        ? FRotator(-10.f, 0, 0)
        : FRotator::ZeroRotator);
}

void ALightSwitchActor::DoToggle()
{
    const bool bNew = !bIsOn;
    PointLight->SetVisibility(bNew);

    UGameplayStatics::PlaySoundAtLocation(
        this,
        bNew ? SwitchOnSound : SwitchOffSound,
        SwitchMesh->GetComponentLocation()
    );

    SwitchMesh->SetRelativeRotation(bNew
        ? FRotator(-10.f, 0, 0)
        : FRotator::ZeroRotator);

    bIsOn = bNew;
}
