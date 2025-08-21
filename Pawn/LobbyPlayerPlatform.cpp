#include "LobbyPlayerPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

ALobbyPlayerPlatform::ALobbyPlayerPlatform()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Mesh setup
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    // Spawn point
    PlayerSpawnTransform = CreateDefaultSubobject<UArrowComponent>(TEXT("PlayerSpawnTransform"));
    PlayerSpawnTransform->SetupAttachment(RootComponent);
    PlayerSpawnTransform->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

    // Platform light
    PlatformLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PlatformLight"));
    PlatformLight->SetupAttachment(RootComponent);
    PlatformLight->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    PlatformLight->SetLightColor(FLinearColor::Blue);
    PlatformLight->SetIntensity(500.0f);
    PlatformLight->SetAttenuationRadius(300.0f);

    // Initialize state
    PC = nullptr;
    CurrentCharacter = nullptr;
    bIsOccupied = false;
    PlatformState = EPlatformState::Empty;
}

void ALobbyPlayerPlatform::BeginPlay()
{
    Super::BeginPlay();

    // Create dynamic material for platform effects
    if (IsValid(Mesh) && Mesh->GetMaterial(0))
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(Mesh->GetMaterial(0), this);
        Mesh->SetMaterial(0, DynamicMaterial);
    }

    UpdatePlatformVisuals();
}

void ALobbyPlayerPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Animate platform when occupied
    if (PlatformState == EPlatformState::Occupied)
    {
        PlatformAnimTime += DeltaTime;
        float PulseValue = FMath::Sin(PlatformAnimTime * 2.0f) * 0.5f + 0.5f;

        if (PlatformLight)
        {
            PlatformLight->SetIntensity(500.0f + PulseValue * 200.0f);
        }

        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), PulseValue);
        }
    }
}

void ALobbyPlayerPlatform::SpawnCharacter(APlayerController* Player)
{
    if (!HasAuthority() || !Player) return;

    PC = Player;

    // Clear existing character
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
        UE_LOG(LogTemp, Error, TEXT("%s: Character spawn failed"), *GetName());
        return;
    }

    CurrentCharacter = NewActor;
    NewActor->SetReplicates(true);

    // Set platform state
    bIsOccupied = true;
    PlatformState = EPlatformState::Occupied;
    PlatformAnimTime = 0.0f;

    UpdatePlatformVisuals();

    UE_LOG(LogTemp, Log, TEXT("%s: Spawned character %s for %s"),
        *GetName(), *GetNameSafe(NewActor), *GetNameSafe(Player));
}

void ALobbyPlayerPlatform::Clear()
{
    if (!HasAuthority()) return;

    if (IsValid(CurrentCharacter))
    {
        // Handle possession cleanup
        if (IsValid(PC))
        {
            if (APawn* Pawn = Cast<APawn>(CurrentCharacter))
            {
                if (PC->GetPawn() == Pawn)
                {
                    PC->UnPossess();
                }
            }
        }

        CurrentCharacter->Destroy();
        CurrentCharacter = nullptr;
    }

    PC = nullptr;
    bIsOccupied = false;
    PlatformState = EPlatformState::Empty;

    UpdatePlatformVisuals();
}

bool ALobbyPlayerPlatform::HasPlayer() const
{
    return bIsOccupied && IsValid(CurrentCharacter);
}

void ALobbyPlayerPlatform::SetPlayerReady(bool bReady)
{
    if (bIsOccupied)
    {
        PlatformState = bReady ? EPlatformState::Ready : EPlatformState::Occupied;
        UpdatePlatformVisuals();
    }
}

void ALobbyPlayerPlatform::UpdatePlatformVisuals()
{
    if (!PlatformLight) return;

    switch (PlatformState)
    {
    case EPlatformState::Empty:
        PlatformLight->SetLightColor(FLinearColor::Blue);
        PlatformLight->SetIntensity(300.0f);
        break;

    case EPlatformState::Occupied:
        PlatformLight->SetLightColor(FLinearColor::White);
        PlatformLight->SetIntensity(500.0f);
        break;

    case EPlatformState::Ready:
        PlatformLight->SetLightColor(FLinearColor::Green);
        PlatformLight->SetIntensity(700.0f);
        break;
    }

    // Update material parameters
    if (DynamicMaterial)
    {
        FLinearColor EmissiveColor;
        switch (PlatformState)
        {
        case EPlatformState::Empty:
            EmissiveColor = FLinearColor::Blue;
            break;
        case EPlatformState::Occupied:
            EmissiveColor = FLinearColor::White;
            break;
        case EPlatformState::Ready:
            EmissiveColor = FLinearColor::Green;
            break;
        }

        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), EmissiveColor);
    }
}

void ALobbyPlayerPlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALobbyPlayerPlatform, bIsOccupied);
    DOREPLIFETIME(ALobbyPlayerPlatform, PlatformState);
}