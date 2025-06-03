// EyeMonster.cpp - Refactored and cleaned version
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
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Controller/GazeLockPlayerController.h"

AEyeMonster::AEyeMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root component
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// Visual components
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	AuraEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AuraEffect"));
	AuraEffect->SetupAttachment(MeshComponent);

	// Spawn volume (detached for fixed world transform)
	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
	SpawnVolume->SetupAttachment(RootComponent);
	SpawnVolume->SetBoxExtent(FVector(1000.f, 1000.f, 500.f));
	SpawnVolume->SetHiddenInGame(true);

	// Default config
	DamageValue = 5.f;
	LookThreshold = 0.8f;
	TraceHeight = 1000.f;
	TraceDepth = 2000.f;
	LookDamageInterval = 1.f;
}

void AEyeMonster::BeginPlay()
{
	Super::BeginPlay();

	// Cache original spawn bounds and detach to preserve world-space
	InitialSpawnCenter = SpawnVolume->GetComponentLocation();
	InitialSpawnExtent = SpawnVolume->GetScaledBoxExtent();
	SpawnVolume->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// Optional: visualize spawn area for debugging
	DrawDebugBox(GetWorld(), InitialSpawnCenter, InitialSpawnExtent, FColor::Red, true, 10.f, 0, 5.f);

	// Place monster and schedule destruction
	RespawnAndDestroy();
}

void AEyeMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ApplyLookDamage(DeltaTime);
	FacePlayer(DeltaTime);

}

void AEyeMonster::RespawnAndDestroy()
{
	// Initial spawn
	SpawnAtRandomLocation();

	// Schedule self-destruction and respawn
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AEyeMonster::HandleSelfDestruct, 10.f, false);
}

void AEyeMonster::HandleSelfDestruct()
{
	// Cache parameters
	UClass* MonsterClass = GetClass();
	const float SavedYawOffset = YawOffset;
	const FVector Center = InitialSpawnCenter;
	const FVector Extent = InitialSpawnExtent;

	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateLambda(
		[this, MonsterClass, SavedYawOffset, Center, Extent]()
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(Center);

			AEyeMonster* NewMonster = GetWorld()->SpawnActorDeferred<AEyeMonster>(
				MonsterClass,
				SpawnTransform
			);
			if (!NewMonster)
				return;

			// Thiết lập lại các giá trị
			NewMonster->InitialSpawnCenter = Center;
			NewMonster->InitialSpawnExtent = Extent;
			NewMonster->YawOffset = SavedYawOffset;

			UGameplayStatics::FinishSpawningActor(NewMonster, SpawnTransform);
			NewMonster->SpawnAtRandomLocation();
		}
	);

	RemoveBloodOverlay();

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, 15.f, false);
	Destroy();
}


void AEyeMonster::SpawnAtRandomLocation()
{
	const FVector Center = InitialSpawnCenter;
	const FVector Extent = InitialSpawnExtent;
	const float X = FMath::FRandRange(Center.X - Extent.X, Center.X + Extent.X);
	const float Y = FMath::FRandRange(Center.Y - Extent.Y, Center.Y + Extent.Y);

	FVector TargetLocation;
	if (GetGroundSpawnLocation(FVector2D(X, Y), TargetLocation))
	{
		SetActorLocation(TargetLocation);
	}
	else
	{
		SetActorLocation(FVector(X, Y, Center.Z - Extent.Z));
	}

	// Immediately face the player
	FacePlayer(0.f);
}

bool AEyeMonster::GetGroundSpawnLocation(const FVector2D& XY, FVector& OutLocation) const
{
	FHitResult Hit;
	const FVector Start{ XY.X, XY.Y, TraceHeight };
	const FVector End{ XY.X, XY.Y, TraceHeight - TraceDepth };
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		OutLocation = Hit.ImpactPoint;
		return true;
	}
	return false;
}

void AEyeMonster::ApplyLookDamage(float DeltaTime)
{
	// --- existing damage logic ---
	if (!CanDamage() || !IsPlayerLookingAtMonster())
	{
		LookDamageTimer = 0.f;
		return;
	}

	LookDamageTimer += DeltaTime;
	if (LookDamageTimer >= LookDamageInterval)
	{
		InflictDamage();
		LookDamageTimer -= LookDamageInterval;
	}
}

bool AEyeMonster::IsPlayerLookingAtMonster() const
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		APawn* Pawn = PC->GetPawn();
		if (Pawn)
		{
			FVector ViewLoc;
			FRotator ViewRot;
			PC->GetPlayerViewPoint(ViewLoc, ViewRot);

			const FVector ToMonster = (GetActorLocation() - ViewLoc).GetSafeNormal();
			const float Dot = FVector::DotProduct(ViewRot.Vector(), ToMonster);

			if (Dot >= LookThreshold)
			{
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(VisibilityTrace), true, this);
				Params.AddIgnoredActor(Pawn);
				if (!GetWorld()->LineTraceSingleByChannel(
					Hit, ViewLoc, GetActorLocation(), ECC_Visibility, Params) ||
					Hit.GetActor() == this)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void AEyeMonster::FacePlayer(float /*DeltaTime*/)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			FVector MyLoc = GetActorLocation();
			FVector PlayerLoc = Pawn->GetActorLocation();
			FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(MyLoc, PlayerLoc);
			LookAt.Pitch = GetActorRotation().Pitch;
			LookAt.Roll = 0.f;
			LookAt.Yaw = FRotator::NormalizeAxis(LookAt.Yaw + YawOffset);
			SetActorRotation(LookAt);
		}
	}
}

void AEyeMonster::InflictDamage()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
		return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UGameplayStatics::ApplyDamage(PlayerPawn, DamageValue, PC, this, UDamageType::StaticClass());

	ShowBloodOverlay(PC);
	ShakeCamera(PC);
	PlayDamageSound();
	UE_LOG(LogTemp, Log, TEXT("EyeMonster inflicted %.2f damage."), DamageValue);
}

void AEyeMonster::ShowBloodOverlay(APlayerController* PC)
{
	if (!BloodOverlayClass || BloodOverlayWidget)
		return;

	BloodOverlayWidget = CreateWidget<UUserWidget>(PC, BloodOverlayClass);
	if (BloodOverlayWidget)
	{
		BloodOverlayWidget->AddToViewport();
		GetWorld()->GetTimerManager().SetTimer(
			BloodOverlayTimerHandle,
			this, &AEyeMonster::RemoveBloodOverlay,
			BloodOverlayDuration, false);
	}
}

void AEyeMonster::ShakeCamera(APlayerController* PC) const
{
	if (CameraShakeClass)
	{
		PC->PlayerCameraManager->StartCameraShake(CameraShakeClass, 1.f);
	}
}

void AEyeMonster::PlayDamageSound() const
{
	if (DamageScreamSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DamageScreamSound, GetActorLocation());
	}
}

void AEyeMonster::RemoveBloodOverlay()
{
	if (BloodOverlayWidget)
	{
		BloodOverlayWidget->RemoveFromParent();
		BloodOverlayWidget = nullptr;
	}
}

void AEyeMonster::DoBlink()
{
	if (bIsBlinking)
		return;

	bIsBlinking = true;
	bIsCoveringEyes = true;

	// Play montage che mắt nếu có
	if (CoverEyesMontage && MeshComponent)
	{
		MeshComponent->PlayAnimation(CoverEyesMontage, false);
	}

	// Schedule finish sau đúng duration của montage hoặc CoverEyesDuration
	const float Delay = CoverEyesMontage
		? CoverEyesMontage->GetPlayLength()
		: CoverEyesDuration;

	GetWorld()->GetTimerManager().SetTimer(
		BlinkTimerHandle,
		this, &AEyeMonster::FinishBlink,
		Delay,
		false
	);
}


void AEyeMonster::FinishBlink()
{
	// Teleport đến vị trí mới (giống cũ)
	const FVector Origin = GetActorLocation();
	const FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, BlinkRadius);
	FVector NewLocation = Origin + RandomOffset;
	SetActorLocation(NewLocation);

	// Hoàn tất trạng thái cover
	bIsCoveringEyes = false;
	bIsBlinking = false;
}

void AEyeMonster::SetIlluminated(bool bOn)
{
	if (bOn)
	{
		if (!bIsCoveringEyes)
		{
			bIsCoveringEyes = true;
			// Play cover-eyes montage nếu có
			if (CoverEyesMontage && MeshComponent)
			{
				MeshComponent->PlayAnimation(CoverEyesMontage, false);
			}
		}
	}
	else
	{
		if (bIsCoveringEyes)
		{
			bIsCoveringEyes = false;
			if (MeshComponent)
			{
				MeshComponent->Stop();
			}
		}
	}
}