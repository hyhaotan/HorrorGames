// LobbyPlayerPlatform.cpp
#include "LobbyPlayerPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

ALobbyPlayerPlatform::ALobbyPlayerPlatform()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    PlayerSpawnTransform = CreateDefaultSubobject<UArrowComponent>(TEXT("PlayerSpawnTransform"));
    PlayerSpawnTransform->SetupAttachment(RootComponent);

    PC = nullptr;
    CurrentCharacter = nullptr;
}

void ALobbyPlayerPlatform::BeginPlay()
{
    Super::BeginPlay();
}

void ALobbyPlayerPlatform::SpawnCharacter(APlayerController* Player)
{
    if (!HasAuthority() || !Player) return;

    PC = Player;

    if (IsValid(CurrentCharacter))
    {
        CurrentCharacter->Destroy();
        CurrentCharacter = nullptr;
    }

    if (!CharacterClass || !IsValid(PlayerSpawnTransform))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Missing CharacterClass or SpawnTransform"), *GetName());
        return;
    }

    const FTransform SpawnTransform = PlayerSpawnTransform->GetComponentTransform();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* NewActor = GetWorld()->SpawnActor<AActor>(CharacterClass, SpawnTransform, SpawnParams);
    if (!IsValid(NewActor))
    {
        UE_LOG(LogTemp, Error, TEXT("%s: Spawn failed"), *GetName());
        return;
    }

    CurrentCharacter = NewActor;
    NewActor->SetReplicates(true);

    UE_LOG(LogTemp, Log, TEXT("%s: Spawned %s for %s"),
        *GetName(), *GetNameSafe(NewActor), *GetNameSafe(Player));

    // Không chuyển camera khỏi ALobbyPawn
    if (IsValid(PC))
    {
        if (APawn* LobbyPawn = PC->GetPawn())
        {
            PC->SetViewTargetWithBlend(LobbyPawn, 0.f);
        }
    }
}

void ALobbyPlayerPlatform::Clear()
{
    if (!HasAuthority()) return;

    if (IsValid(CurrentCharacter))
    {
        if (IsValid(PC))
        {
            if (APawn* Pawn = Cast<APawn>(CurrentCharacter))
            {
                if (PC->GetPawn() == Pawn)
                {
                    PC->UnPossess();
                    PC->SetViewTarget(this);
                }
            }
        }
        CurrentCharacter->Destroy();
        CurrentCharacter = nullptr;
    }
    PC = nullptr;
}