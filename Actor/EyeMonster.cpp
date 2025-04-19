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
    LookDamageInterval = 1.0f;
    LookDamageTimer = 0.0f;
    bHasInflictedDamage = false;
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
    // 1) Random vị trí trong volume gốc
    const FVector Center = InitialSpawnCenter;
    const FVector Extent = InitialSpawnExtent;

    float RandX = FMath::FRandRange(Center.X - Extent.X, Center.X + Extent.X);
    float RandY = FMath::FRandRange(Center.Y - Extent.Y, Center.Y + Extent.Y);

    FVector GroundLoc;
    if (GetGroundSpawnLocation(FVector2D(RandX, RandY), GroundLoc))
    {
        SetActorLocation(GroundLoc);
    }
    else
    {
        SetActorLocation(FVector(RandX, RandY, Center.Z - Extent.Z));
    }

    // 2) Tính rotation để quay mặt về player
    FacePlayerInstant();
}


void AEyeMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    HandleLookDamage(DeltaTime);
    RotateToPlayer(DeltaTime);
}

void AEyeMonster::HandleLookDamage(float DeltaTime)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
    if (!PC || !PlayerPawn) return;

    // 1) Lấy viewpoint
    FVector PlayerLocation;
    FRotator PlayerRotation;
    PC->GetPlayerViewPoint(PlayerLocation, PlayerRotation);

    // 2) Tính góc nhìn
    FVector ToMonster = (GetActorLocation() - PlayerLocation).GetSafeNormal();
    FVector PlayerForward = PlayerRotation.Vector();
    float   Dot = FVector::DotProduct(PlayerForward, ToMonster);

    if (Dot >= LookThreshold)
    {
        // 3) Line‑trace kiểm tra vật cản (visibility)
        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(VisibilityTrace), true, this);
        Params.AddIgnoredActor(PlayerPawn);
        // Trace từ camera đến monster
        bool bBlocked = GetWorld()->LineTraceSingleByChannel(
            Hit,
            PlayerLocation,
            GetActorLocation(),
            ECC_Visibility,
            Params
        ) && Hit.GetActor() != this;

        if (bBlocked)
        {
            // Có vật chắn → reset timer, không gây damage
            LookDamageTimer = 0.0f;
            return;
        }

        // 4) Không bị che khuất → cộng dồn ΔTime
        LookDamageTimer += DeltaTime;
        if (LookDamageTimer >= LookDamageInterval)
        {
            InflictDamage();
            LookDamageTimer -= LookDamageInterval;
        }
    }
    else
    {
        // Nhìn đi chỗ khác → reset
        LookDamageTimer = 0.0f;
    }
}

void AEyeMonster::FacePlayerInstant()
{
    // 1) Lấy PlayerController + Pawn
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            // 2) Tính LookAt từ monster tới player
            const FVector MyLoc = GetActorLocation();
            const FVector PlayerLoc = Pawn->GetActorLocation();
            FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MyLoc, PlayerLoc);

            // 3) Giữ nguyên pitch hiện tại, đặt roll = 0, áp offset yaw
            LookAtRot.Pitch = GetActorRotation().Pitch;
            LookAtRot.Roll = 0.f;
            LookAtRot.Yaw = FRotator::NormalizeAxis(LookAtRot.Yaw + YawOffset);

            // 4) Apply rotation
            SetActorRotation(LookAtRot);
        }
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
    FacePlayerInstant();
}


void AEyeMonster::DestroySelf()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        Destroy();
        return;
    }

    // 1) Lưu lại class, offset yaw, và volume gốc
    UClass* MonsterClass = GetClass();
    float SavedYawOffset = YawOffset;
    FVector OriginalCenter = InitialSpawnCenter;
    FVector OriginalExtent = InitialSpawnExtent;

    // 2) Tạo delegate respawn
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateLambda(
        [World, MonsterClass, OriginalCenter, OriginalExtent, SavedYawOffset]() mutable
        {
            // Tạo transform ban đầu tại center gốc (chỉ để spawn deferred, vị trí thực sẽ đc SetupSpawnLocation)
            FTransform SpawnTransform;
            SpawnTransform.SetLocation(OriginalCenter);

            // Spawn deferred để chúng ta còn thiết lập biến trước BeginPlay
            AEyeMonster* NewMonster = World->SpawnActorDeferred<AEyeMonster>(
                MonsterClass,
                SpawnTransform,
                nullptr,
                nullptr,
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn
            );
            if (NewMonster)
            {
                // 3) Gán lại volume gốc và yaw-offset
                NewMonster->InitialSpawnCenter = OriginalCenter;
                NewMonster->InitialSpawnExtent = OriginalExtent;
                NewMonster->YawOffset = SavedYawOffset;

                // 4) Finish spawning → BeginPlay chạy, rồi ta random thêm
                UGameplayStatics::FinishSpawningActor(NewMonster, SpawnTransform);

                // 5) Random vị trí mới trong volume
                NewMonster->SetupSpawnLocation();
            }
        }
    );

    // 6) Đặt timer để respawn
    World->GetTimerManager().SetTimer(
        RespawnTimerHandle,
        RespawnDelegate,
        15.0f,
        false
    );

    // Cuối cùng destroy instance cũ
    Destroy();
}
