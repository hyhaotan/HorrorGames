#include "EyeMonster.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AEyeMonster::AEyeMonster()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    // Mesh & effects
    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(SceneRoot);

    AuraEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AuraEffect"));
    AuraEffect->SetupAttachment(MeshComponent);

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    SpawnVolume->SetupAttachment(RootComponent);
    SpawnVolume->SetBoxExtent(FVector(1000, 1000, 500));
    SpawnVolume->SetHiddenInGame(false);
    SpawnVolume->bDrawOnlyIfSelected = true;

    // Defaults
    DamageValue = 5.0f;
    LookThreshold = 0.8f;
    TraceHeight = 1000.f;
    TraceDepth = 2000.f;
    RotationSpeed = 2.5f;
}

bool AEyeMonster::GetGroundSpawnLocation(const FVector2D& XY, FVector& OutLocation)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    FVector Start(XY.X, XY.Y, TraceHeight);
    FVector End(XY.X, XY.Y, TraceHeight - TraceDepth);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        OutLocation = Hit.ImpactPoint;
        return true;
    }
    return false;
}

void AEyeMonster::BeginPlay()
{
    Super::BeginPlay();

    // 1) Ghi nhớ world‐transform gốc của spawn volume
    InitialSpawnCenter = SpawnVolume->GetComponentLocation();
    InitialSpawnExtent = SpawnVolume->GetScaledBoxExtent();

    // 2) Detach để box không dịch chuyển khi monster thay đổi vị trí
    SpawnVolume->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

    // VẪN vẽ debug box ở vị trí gốc
    DrawDebugBox(
        GetWorld(),
        InitialSpawnCenter,
        InitialSpawnExtent,
        FQuat::Identity,
        FColor::Red, true, 10.f, 0, 5.f
    );

    // Spawn lần đầu (cũng dùng hàm này, sẽ random ngẫu nhiên trong volume gốc)
    SetupSpawnLocation();

    // Hủy bản thân sau 10s
    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle, this,
        &AEyeMonster::DestroySelf, 10.f, false);
}

void AEyeMonster::SetupSpawnLocation()
{
    // Lấy center + extent gốc
    const FVector Center = InitialSpawnCenter;
    const FVector Extent = InitialSpawnExtent;

    // Random trong box gốc
    float RandX = FMath::FRandRange(Center.X - Extent.X, Center.X + Extent.X);
    float RandY = FMath::FRandRange(Center.Y - Extent.Y, Center.Y + Extent.Y);

    FVector GroundLoc;
    if (GetGroundSpawnLocation(FVector2D(RandX, RandY), GroundLoc))
    {
        SetActorLocation(GroundLoc);
    }
    else
    {
        // fallback xuống đáy volume nếu line trace fail
        SetActorLocation(FVector(RandX, RandY, Center.Z - Extent.Z));
    }
}

void AEyeMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    HandleLookDamage(DeltaTime);
    RotateToPlayer(DeltaTime);
}

void AEyeMonster::HandleLookDamage(float DeltaTime)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->GetPawn()) return;

    // Lấy vị trí & hướng camera
    FVector PlayerLoc; FRotator PlayerRot;
    PC->GetPlayerViewPoint(PlayerLoc, PlayerRot);

    FVector ToMonster = (GetActorLocation() - PlayerLoc).GetSafeNormal();
    float Dot = FVector::DotProduct(PlayerRot.Vector(), ToMonster);

    // Kiểm tra vật cản
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(PC->GetPawn());
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, PlayerLoc, GetActorLocation(), ECC_Visibility, Params);
    bool bBlocked = bHit && Hit.GetActor() != this;

    if (Dot >= LookThreshold && !bBlocked)
    {
        if (!GetWorldTimerManager().IsTimerActive(LookDamageTimerHandle))
        {
            // Phát ngay lần đầu
         /*   InflictDamage();*/

            // Sau đó lặp mỗi 1s
            GetWorldTimerManager().SetTimer(
                LookDamageTimerHandle,
                this,
                &AEyeMonster::InflictDamage,
                1.0f,    // interval
                true     // looping
            );
        }
    }
    else
    {
        // Hủy timer khi người chơi không nhìn
        GetWorldTimerManager().ClearTimer(LookDamageTimerHandle);
    }
}


void AEyeMonster::InflictDamage()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;

    // Áp dụng damage
    UGameplayStatics::ApplyDamage(
        PlayerPawn,
        DamageValue,
        UGameplayStatics::GetPlayerController(this, 0),
        this,
        UDamageType::StaticClass()
    );

    // Camera shake
    if (CameraShakeClass)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            PC->PlayerCameraManager->StartCameraShake(CameraShakeClass, 1.0f);
        }
    }

    // Sound
    if (DamageScreamSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DamageScreamSound, GetActorLocation());
    }

    UE_LOG(LogTemp, Log, TEXT("EyeMonster inflicted %f damage."), DamageValue);
}


void AEyeMonster::RotateToPlayer(float DeltaTime)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    FVector MyLoc = GetActorLocation();
    FVector TargetLoc = PC->GetPawn()->GetActorLocation();
    FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetLoc);

    DesiredRot.Pitch = GetActorRotation().Pitch;
    DesiredRot.Roll = 0.f;
    // Normalize yaw after applying offset
    DesiredRot.Yaw = FRotator::NormalizeAxis(DesiredRot.Yaw + YawOffset);

    SetActorRotation(DesiredRot);
}

void AEyeMonster::DestroySelf()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        Destroy();
        return;
    }

    // Save class and current location/properties for respawn
    UClass* MonsterClass = GetClass();
    FVector SpawnLocation = GetActorLocation();
    float   SavedYawOffset = YawOffset;

    // Setup delegate to respawn after delay
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateLambda(
        [World, MonsterClass, SpawnLocation, SavedYawOffset]() mutable
        {
            // Get player location at respawn time
            FVector PlayerLoc = FVector::ZeroVector;
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    PlayerLoc = Pawn->GetActorLocation();
                }
            }

            // Calculate look-at rotation
            FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, PlayerLoc);
            LookAtRot.Pitch = 0.f;
            LookAtRot.Roll = 0.f;
            LookAtRot.Yaw = FRotator::NormalizeAxis(LookAtRot.Yaw + SavedYawOffset);

            // Deferred spawn to set properties before BeginPlay
            FTransform SpawnTransform(LookAtRot, SpawnLocation);
            AEyeMonster* NewMonster = World->SpawnActorDeferred<AEyeMonster>(
                MonsterClass,
                SpawnTransform,
                nullptr,
                nullptr,
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn
            );
            if (NewMonster)
            {
                // Transfer offsets and initial center
                NewMonster->YawOffset = SavedYawOffset;
                NewMonster->InitialSpawnCenter = SpawnLocation;
                UGameplayStatics::FinishSpawningActor(NewMonster, SpawnTransform);
            }
        }
    );

    World->GetTimerManager().SetTimer(
        RespawnTimerHandle,
        RespawnDelegate,
        15.0f,
        false
    );

    // Destroy current instance
    Destroy();
}