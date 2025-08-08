#include "Door.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"

ADoor::ADoor()
{
    // Set specific default values for this door type
    DoorRotateAngle = 90.0f;

    // You can customize the mesh, materials, etc. here
    // DoorMesh->SetStaticMesh(YourSpecificDoorMesh);
}

void ADoor::OnDoorInteraction_Implementation(AHorrorGameCharacter* Player)
{
    // Call parent implementation first if needed
    Super::OnDoorInteraction_Implementation(Player);

    // Add custom flip-flop door behavior here
    UE_LOG(LogTemp, Log, TEXT("Flip-flop door interaction with player: %s"),
        Player ? *Player->GetName() : TEXT("Unknown"));

    if (!bIsDoorClosed) bIsDoorClosed = false;
}

bool ADoor::CanOpenDoor_Implementation(AHorrorGameCharacter* Player)
{
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("No player for door interaction"));
        return false;
    }

    if (bIsAnimating)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door is currently animating"));
        return false;
    }

    // For flip-flop doors, always allow interaction if basic conditions are met
    return true;
}

void ADoor::PlayDoorAnimation_Implementation()
{
    // Use parent implementation for basic animation
    Super::PlayDoorAnimation_Implementation();

    // Add any additional visual effects specific to this door
    // Example: particle effects, additional sounds, etc.
    UE_LOG(LogTemp, Log, TEXT("Playing custom flip-flop door animation"));
}

FRotator ADoor::CalculateDoorRotation(float AnimationValue)
{
    // Use parent implementation as base
    FRotator BaseRotation = Super::CalculateDoorRotation(AnimationValue);

    // Add any custom rotation modifications if needed
    // For example, slight wobble, different swing pattern, etc.

    return BaseRotation;
}