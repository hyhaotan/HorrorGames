#include "LightSwitchActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ALightSwitchActor::ALightSwitchActor()
{
    PrimaryActorTick.bCanEverTick = true;

	SpotLight->SetVisibility(false);

    SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
    SwitchMesh->SetupAttachment(LightMesh);
}

void ALightSwitchActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
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
    SpotLight->SetVisibility(bIsOn);
    SwitchMesh->SetRelativeRotation(bIsOn
        ? FRotator(-10.f, 0, 0)
        : FRotator::ZeroRotator);
}

void ALightSwitchActor::DoToggle()
{
    const bool bNew = !bIsOn;
    SpotLight->SetVisibility(bNew);

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
