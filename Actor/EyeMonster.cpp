// EyeMonster.cpp - Enhanced Horror Version
#include "EyeMonster.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Actor/Component/SanityComponent.h"
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

	// Enhanced Horror Configuration
	LookThreshold = 0.7f; // Easier to trigger
	TraceHeight = 1000.f;
	TraceDepth = 2000.f;

	// Sanity damage settings
	BaseSanityDamage = 5.0f;
	MaxSanityDamage = 15.0f;
	SanityDamageInterval = 0.8f; // Faster damage tick

	// Staring mechanics
	StareIntensifyRate = 1.5f;
	MaxStareTime = 5.0f;

	// Proximity fear
	ProximityFearRadius = 500.0f;
	ProximityFearDamage = 5.0f;

	// Blink mechanics
	BlinkCooldown = 8.0f;
	BlinkTeleportRadius = 300.0f;

	// State variables
	bIsStaring = false;
	CurrentStareTime = 0.0f;
	CurrentStareIntensity = 1.0f;
	bPlayerAware = false;
	LastBlinkTime = 0.0f;

	// Visual effects
	bEyesGlowing = false;
	EyeGlowIntensity = 1.0f;
}

void AEyeMonster::BeginPlay()
{
	Super::BeginPlay();

	// Cache player components
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn) CachedSanityComponent = PlayerPawn->FindComponentByClass<USanityComponent>();

	// Initialize spawn system
	InitialSpawnCenter = SpawnVolume->GetComponentLocation();
	InitialSpawnExtent = SpawnVolume->GetScaledBoxExtent();
	SpawnVolume->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// Set initial state
	SetCurrentBehaviorState(EEyeMonsterState::Lurking);

	DrawSpawnDebug();
	RespawnAndDestroy();
}

void AEyeMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateBehaviorState(DeltaTime);
	ProcessStaring(DeltaTime);
	CheckProximityFear(DeltaTime);
	UpdateVisualEffects(DeltaTime);
	FacePlayer(DeltaTime);
}

void AEyeMonster::UpdateBehaviorState(float DeltaTime)
{
	switch (CurrentState)
	{
	case EEyeMonsterState::Lurking:
		HandleLurkingState(DeltaTime);
		break;

	case EEyeMonsterState::Staring:
		HandleStaringState(DeltaTime);
		break;

	case EEyeMonsterState::Aggressive:
		HandleAggressiveState(DeltaTime);
		break;

	case EEyeMonsterState::Retreating:
		HandleRetreatingState(DeltaTime);
		break;
	}
}

void AEyeMonster::HandleLurkingState(float DeltaTime)
{
	// Monster đang ẩn nấp, chờ player nhìn thấy
	if (IsPlayerLookingAtMonster())
	{
		SetCurrentBehaviorState(EEyeMonsterState::Staring);
		OnPlayerStartedLooking();
	}

	// Random blink/teleport để tạo surprise
	if (ShouldRandomBlink())
	{
		PerformStealthBlink();
	}
}

void AEyeMonster::HandleStaringState(float DeltaTime)
{
	if (!IsPlayerLookingAtMonster())
	{
		OnPlayerStoppedLooking();

		// Decide next state based on how long they stared
		if (CurrentStareTime >= MaxStareTime * 0.7f)
		{
			SetCurrentBehaviorState(EEyeMonsterState::Aggressive);
		}
		else
		{
			SetCurrentBehaviorState(EEyeMonsterState::Lurking);
		}
		return;
	}

	// Continuing to stare - increase intensity
	CurrentStareTime += DeltaTime;
	CurrentStareIntensity = FMath::Clamp(
		1.0f + (CurrentStareTime * StareIntensifyRate),
		1.0f,
		3.0f
	);

	// Apply escalating sanity damage
	ApplyStaringSanityDamage(DeltaTime);

	// Transition to aggressive if stared at too long
	if (CurrentStareTime >= MaxStareTime)
	{
		SetCurrentBehaviorState(EEyeMonsterState::Aggressive);
		TriggerAggressiveResponse();
	}
}

void AEyeMonster::HandleAggressiveState(float DeltaTime)
{
	// Monster trở nên hostile, gây damage cao hơn
	if (IsPlayerLookingAtMonster())
	{
		ApplyAggressiveSanityDamage(DeltaTime);

		// Rapid blinking/teleporting
		if (CanPerformBlink())
		{
			PerformAggressiveBlink();
		}
	}
	else
	{
		// Cooldown period
		AggressiveCooldownTimer += DeltaTime;
		if (AggressiveCooldownTimer >= 3.0f)
		{
			SetCurrentBehaviorState(EEyeMonsterState::Retreating);
		}
	}
}

void AEyeMonster::HandleRetreatingState(float DeltaTime)
{
	// Monster rút lui, teleport xa và reset
	RetreatCooldownTimer += DeltaTime;

	if (RetreatCooldownTimer >= 2.0f)
	{
		PerformRetreatTeleport();
		SetCurrentBehaviorState(EEyeMonsterState::Lurking);
		ResetStareData();
	}
}

void AEyeMonster::ProcessStaring(float DeltaTime)
{
	bool bCurrentlyLooking = IsPlayerLookingAtMonster();

	if (bCurrentlyLooking != bIsStaring)
	{
		if (bCurrentlyLooking)
		{
			OnPlayerStartedLooking();
		}
		else
		{
			OnPlayerStoppedLooking();
		}
		bIsStaring = bCurrentlyLooking;
	}
}

void AEyeMonster::ApplyStaringSanityDamage(float DeltaTime)
{
	if (!CachedSanityComponent || !IsPlayerLookingAtMonster())
		return;

	SanityDamageTimer += DeltaTime;
	if (SanityDamageTimer >= SanityDamageInterval)
	{
		// Calculate damage based on stare intensity and proximity
		float Distance = FVector::Dist(GetActorLocation(), GetPlayerLocation());
		float ProximityMultiplier = FMath::GetMappedRangeValueClamped(
			FVector2D(100.0f, 1000.0f),
			FVector2D(2.0f, 1.0f),
			Distance
		);

		float FinalDamage = BaseSanityDamage * CurrentStareIntensity * ProximityMultiplier;
		FinalDamage = FMath::Clamp(FinalDamage, BaseSanityDamage, MaxSanityDamage);

		CachedSanityComponent->OnSeeGhost(FinalDamage);

		// Visual feedback
		TriggerSanityDamageEffects(FinalDamage);

		SanityDamageTimer = 0.0f;

		UE_LOG(LogTemp, Log, TEXT("EyeMonster: Applied %.1f sanity damage (Intensity: %.1f, Proximity: %.1f)"),
			FinalDamage, CurrentStareIntensity, ProximityMultiplier);
	}
}

void AEyeMonster::ApplyAggressiveSanityDamage(float DeltaTime)
{
	if (!CachedSanityComponent)
		return;

	AggressiveDamageTimer += DeltaTime;
	if (AggressiveDamageTimer >= SanityDamageInterval * 0.5f) // Faster damage in aggressive state
	{
		float AggressiveDamage = MaxSanityDamage * 1.5f; // Even higher damage
		CachedSanityComponent->OnSeeGhost(AggressiveDamage);

		// Intense effects
		TriggerAggressiveEffects();

		AggressiveDamageTimer = 0.0f;
	}
}

void AEyeMonster::CheckProximityFear(float DeltaTime)
{
	if (!CachedSanityComponent)
		return;

	float Distance = FVector::Dist(GetActorLocation(), GetPlayerLocation());
	if (Distance <= ProximityFearRadius)
	{
		ProximityFearTimer += DeltaTime;
		if (ProximityFearTimer >= 2.0f) // Proximity fear every 2 seconds
		{
			float ProximityDamage = ProximityFearDamage * (ProximityFearRadius / Distance);
			CachedSanityComponent->OnSeeGhost(ProximityDamage);

			TriggerProximityFearEffect();
			ProximityFearTimer = 0.0f;
		}
	}
	else
	{
		ProximityFearTimer = 0.0f;
	}
}

void AEyeMonster::OnPlayerStartedLooking()
{
	UE_LOG(LogTemp, Log, TEXT("EyeMonster: Player started looking"));

	// Reset stare tracking
	CurrentStareTime = 0.0f;
	CurrentStareIntensity = 1.0f;

	// Visual response
	StartEyeGlow();
	PlayStareSound();

	// Particle effects
	if (AuraEffect)
	{
		AuraEffect->SetFloatParameter(TEXT("Intensity"), 1.5f);
	}
}

void AEyeMonster::OnPlayerStoppedLooking()
{
	UE_LOG(LogTemp, Log, TEXT("EyeMonster: Player stopped looking (stared for %.1fs)"), CurrentStareTime);

	// Visual response
	StopEyeGlow();

	// Reset aura
	if (AuraEffect)
	{
		AuraEffect->SetFloatParameter(TEXT("Intensity"), 1.0f);
	}

	// Chance to blink away if stared at long enough
	if (CurrentStareTime >= 2.0f && CanPerformBlink())
	{
		PerformEscapeBlink();
	}
}

void AEyeMonster::TriggerAggressiveResponse()
{
	UE_LOG(LogTemp, Warning, TEXT("EyeMonster: Entering aggressive mode!"));

	// Intense screen effects
	ShowIntenseBloodOverlay();

	// Aggressive camera shake
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		ShakeCamera(PC, 2.0f); // Stronger shake
	}

	// Play aggressive sound
	PlayAggressiveSound();

	// Spawn additional visual effects
	SpawnAggressiveEffects();
}

void AEyeMonster::PerformStealthBlink()
{
	// Subtle repositioning when not being watched
	if (!IsPlayerLookingAtMonster() && CanPerformBlink())
	{
		FVector NewPosition = FindStealthyPosition();
		if (NewPosition != FVector::ZeroVector)
		{
			SetActorLocation(NewPosition);
			LastBlinkTime = GetWorld()->GetTimeSeconds();

			// Subtle audio cue
			PlayStealthSound();
		}
	}
}

void AEyeMonster::PerformAggressiveBlink()
{
	// Aggressive teleporting - closer to player or to unexpected angles
	FVector PlayerLoc = GetPlayerLocation();
	FVector NewPosition = FindAggressivePosition(PlayerLoc);

	if (NewPosition != FVector::ZeroVector)
	{
		// Brief invisibility
		SetActorHiddenInGame(true);

		FTimerDelegate BlinkDelegate = FTimerDelegate::CreateLambda([this, NewPosition]()
			{
				SetActorLocation(NewPosition);
				SetActorHiddenInGame(false);

				// Dramatic reappearance effects
				TriggerBlinkEffects();
			});

		GetWorld()->GetTimerManager().SetTimer(BlinkEffectTimer, BlinkDelegate, 0.3f, false);
		LastBlinkTime = GetWorld()->GetTimeSeconds();
	}
}

void AEyeMonster::PerformEscapeBlink()
{
	// Blink away from player when they stop looking
	FVector FarPosition = FindEscapePosition();
	if (FarPosition != FVector::ZeroVector)
	{
		SetActorLocation(FarPosition);
		LastBlinkTime = GetWorld()->GetTimeSeconds();
		PlayEscapeSound();
	}
}

void AEyeMonster::PerformRetreatTeleport()
{
	// Full retreat - teleport to far corner of spawn area
	FVector RetreatPos = FindRetreatPosition();
	SetActorLocation(RetreatPos);
	PlayRetreatSound();
}

FVector AEyeMonster::FindStealthyPosition()
{
	// Find position behind player or to the side
	FVector PlayerLoc = GetPlayerLocation();
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return FVector::ZeroVector;

	FVector ViewLoc;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewLoc, ViewRot);

	// Behind player
	FVector BehindDir = -ViewRot.Vector();
	FVector TestPos = PlayerLoc + BehindDir * FMath::RandRange(200.0f, 400.0f);

	FVector GroundPos;
	if (GetGroundSpawnLocation(FVector2D(TestPos.X, TestPos.Y), GroundPos))
	{
		return GroundPos;
	}

	return FVector::ZeroVector;
}

FVector AEyeMonster::FindAggressivePosition(const FVector& PlayerLocation)
{
	// Find position that's threatening - closer or at unexpected angles
	const float MinDistance = 150.0f;
	const float MaxDistance = 300.0f;

	for (int32 i = 0; i < 8; ++i)
	{
		float Angle = (360.0f / 8.0f) * i;
		FVector Direction = FRotator(0, Angle, 0).Vector();
		float Distance = FMath::RandRange(MinDistance, MaxDistance);
		FVector TestPos = PlayerLocation + Direction * Distance;

		FVector GroundPos;
		if (GetGroundSpawnLocation(FVector2D(TestPos.X, TestPos.Y), GroundPos))
		{
			return GroundPos;
		}
	}

	return GetActorLocation(); // Fallback
}

FVector AEyeMonster::FindEscapePosition()
{
	// Find position far from player
	FVector PlayerLoc = GetPlayerLocation();
	const float EscapeDistance = 600.0f;

	for (int32 i = 0; i < 12; ++i)
	{
		float Angle = (360.0f / 12.0f) * i;
		FVector Direction = FRotator(0, Angle, 0).Vector();
		FVector TestPos = PlayerLoc + Direction * EscapeDistance;

		FVector GroundPos;
		if (GetGroundSpawnLocation(FVector2D(TestPos.X, TestPos.Y), GroundPos))
		{
			return GroundPos;
		}
	}

	return InitialSpawnCenter; // Fallback to spawn center
}

FVector AEyeMonster::FindRetreatPosition()
{
	// Return to far corner of spawn area
	const FVector Center = InitialSpawnCenter;
	const FVector Extent = InitialSpawnExtent;

	// Pick a random corner
	float X = FMath::RandBool() ? Center.X + Extent.X * 0.8f : Center.X - Extent.X * 0.8f;
	float Y = FMath::RandBool() ? Center.Y + Extent.Y * 0.8f : Center.Y - Extent.Y * 0.8f;

	FVector GroundPos;
	if (GetGroundSpawnLocation(FVector2D(X, Y), GroundPos))
	{
		return GroundPos;
	}

	return Center;
}

bool AEyeMonster::CanPerformBlink() const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastBlinkTime) >= BlinkCooldown;
}

bool AEyeMonster::ShouldRandomBlink() const
{
	if (!CanPerformBlink()) return false;

	// 5% chance per second to randomly blink when lurking
	return FMath::RandRange(0.0f, 1.0f) < 0.05f;
}

void AEyeMonster::SetCurrentBehaviorState(EEyeMonsterState NewState)
{
	if (CurrentState == NewState) return;

	EEyeMonsterState OldState = CurrentState;
	CurrentState = NewState;

	// Reset timers for new state
	switch (NewState)
	{
	case EEyeMonsterState::Aggressive:
		AggressiveCooldownTimer = 0.0f;
		AggressiveDamageTimer = 0.0f;
		break;
	case EEyeMonsterState::Retreating:
		RetreatCooldownTimer = 0.0f;
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("EyeMonster: State changed from %d to %d"), (int32)OldState, (int32)NewState);
}

void AEyeMonster::ResetStareData()
{
	CurrentStareTime = 0.0f;
	CurrentStareIntensity = 1.0f;
	SanityDamageTimer = 0.0f;
	bIsStaring = false;
}

FVector AEyeMonster::GetPlayerLocation() const
{
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return PlayerPawn->GetActorLocation();
	}
	return FVector::ZeroVector;
}

// Enhanced visual and audio effects
void AEyeMonster::StartEyeGlow()
{
	bEyesGlowing = true;
	EyeGlowIntensity = 1.0f;

	// Set material parameters for glowing eyes
	if (MeshComponent)
	{
		MeshComponent->SetScalarParameterValueOnMaterials(TEXT("EyeGlow"), EyeGlowIntensity);
	}
}

void AEyeMonster::StopEyeGlow()
{
	bEyesGlowing = false;
	EyeGlowIntensity = 0.0f;

	if (MeshComponent)
	{
		MeshComponent->SetScalarParameterValueOnMaterials(TEXT("EyeGlow"), 0.0f);
	}
}

void AEyeMonster::UpdateVisualEffects(float DeltaTime)
{
	if (bEyesGlowing)
	{
		// Pulsing eye glow based on stare intensity
		float PulseFreq = 2.0f * CurrentStareIntensity;
		float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * PulseFreq) * 0.3f + 0.7f;
		float FinalGlow = EyeGlowIntensity * PulseValue * CurrentStareIntensity;

		if (MeshComponent)
		{
			MeshComponent->SetScalarParameterValueOnMaterials(TEXT("EyeGlow"), FinalGlow);
		}
	}
}

void AEyeMonster::TriggerSanityDamageEffects(float DamageAmount)
{
	// Screen effects based on damage amount
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		float EffectIntensity = FMath::GetMappedRangeValueClamped(
			FVector2D(BaseSanityDamage, MaxSanityDamage),
			FVector2D(0.3f, 1.0f),
			DamageAmount
		);

		ShowBloodOverlay(PC, EffectIntensity);
		ShakeCamera(PC, EffectIntensity);
	}

	PlayDamageSound();
}

// Audio system
void AEyeMonster::PlayStareSound()
{
	// Subtle creepy sound when player starts looking
	if (StareStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StareStartSound, GetActorLocation(), 0.5f);
	}
}

void AEyeMonster::PlayAggressiveSound()
{
	// Intense sound for aggressive state
	if (AggressiveSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AggressiveSound, GetActorLocation(), 1.0f);
	}
}

void AEyeMonster::PlayStealthSound()
{
	// Very quiet sound for stealth movement
	if (StealthBlinkSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StealthBlinkSound, GetActorLocation(), 0.2f);
	}
}

void AEyeMonster::PlayEscapeSound()
{
	// Sound for escaping when player looks away
	if (EscapeBlinkSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EscapeBlinkSound, GetActorLocation(), 0.7f);
	}
}

void AEyeMonster::PlayRetreatSound()
{
	// Sound for full retreat
	if (RetreatSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RetreatSound, GetActorLocation(), 0.8f);
	}
}

// Keep existing methods but enhance them
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

void AEyeMonster::ShowBloodOverlay(APlayerController* PC, float Intensity)
{
	if (!BloodOverlayClass || BloodOverlayWidget)
		return;

	BloodOverlayWidget = CreateWidget<UUserWidget>(PC, BloodOverlayClass);
	if (BloodOverlayWidget)
	{
		BloodOverlayWidget->AddToViewport();

		// Set opacity based on intensity
		BloodOverlayWidget->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, Intensity));

		float Duration = BloodOverlayDuration * Intensity;
		GetWorld()->GetTimerManager().SetTimer(
			BloodOverlayTimerHandle,
			this, &AEyeMonster::RemoveBloodOverlay,
			Duration, false);
	}
}

void AEyeMonster::ShakeCamera(APlayerController* PC, float Intensity) const
{
	if (CameraShakeClass)
	{
		PC->PlayerCameraManager->StartCameraShake(CameraShakeClass, Intensity);
	}
}

// Rest of the existing methods remain the same...
void AEyeMonster::DrawSpawnDebug() const
{
#if WITH_EDITOR
	DrawDebugBox(GetWorld(), InitialSpawnCenter, InitialSpawnExtent, FColor::Red, true, 10.f);
#endif
}

void AEyeMonster::RespawnAndDestroy()
{
	SpawnAtRandomLocation();
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AEyeMonster::HandleSelfDestruct, 15.f, false);
}

void AEyeMonster::HandleSelfDestruct()
{
	TWeakObjectPtr<AEyeMonster> WeakThis(this);
	UClass* MonsterClass = GetClass();
	const float SavedYawOffset = YawOffset;
	const FVector Center = InitialSpawnCenter;
	const FVector Extent = InitialSpawnExtent;

	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateLambda(
		[WeakThis, MonsterClass, SavedYawOffset, Center, Extent]()
		{
			if (!WeakThis.IsValid()) return;
			UWorld* World = WeakThis->GetWorld();
			if (!World) return;

			FTransform SpawnTransform;
			SpawnTransform.SetLocation(Center);

			AEyeMonster* NewMonster = World->SpawnActorDeferred<AEyeMonster>(
				MonsterClass,
				SpawnTransform
			);
			if (!NewMonster) return;

			NewMonster->InitialSpawnCenter = Center;
			NewMonster->InitialSpawnExtent = Extent;
			NewMonster->YawOffset = SavedYawOffset;

			UGameplayStatics::FinishSpawningActor(NewMonster, SpawnTransform);
			NewMonster->SpawnAtRandomLocation();
		}
	);

	RemoveBloodOverlay();
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, 20.f, false);
	Destroy();
}

void AEyeMonster::SpawnAtRandomLocation()
{
	const FVector Center = InitialSpawnCenter;
	const FVector Extent = InitialSpawnExtent;
	const int32 MaxAttempts = 10;

	for (int32 i = 0; i < MaxAttempts; ++i)
	{
		const float X = FMath::FRandRange(Center.X - Extent.X, Center.X + Extent.X);
		const float Y = FMath::FRandRange(Center.Y - Extent.Y, Center.Y + Extent.Y);

		FVector TargetLocation;
		if (GetGroundSpawnLocation(FVector2D(X, Y), TargetLocation))
		{
			ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0);
			if (PlayerChar)
			{
				FVector PlayerViewLoc = PlayerChar->GetActorLocation();
				FHitResult Hit;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);

				if (!GetWorld()->LineTraceSingleByChannel(Hit, TargetLocation + FVector(0, 0, 50), PlayerViewLoc, ECC_Visibility, Params)
					|| Hit.GetActor() == PlayerChar)
				{
					SetActorLocation(TargetLocation);
					FacePlayer(0.f);
					return;
				}
			}
		}
	}

	// Fallback: use center-bottom if all fails
	SetActorLocation(Center - FVector(0, 0, Extent.Z));
	FacePlayer(0.f);
}

bool AEyeMonster::GetGroundSpawnLocation(const FVector2D& XY, FVector& OutLocation) const
{
	FHitResult Hit;
	const FVector Start{ XY.X, XY.Y, TraceHeight };
	const FVector End{ XY.X, XY.Y, TraceHeight - TraceDepth };
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		OutLocation = Hit.ImpactPoint + FVector(0, 0, 5.0f);

		const float SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector)));
		if (SurfaceAngle < 30.f)
		{
			return true;
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

void AEyeMonster::ShowIntenseBloodOverlay()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		ShowBloodOverlay(PC, 1.5f); // Maximum intensity
	}
}

void AEyeMonster::TriggerAggressiveEffects()
{
	// Spawn aggressive particle effects
	SpawnAggressiveEffects();

	// Play aggressive sound
	PlayAggressiveSound();

	// Intense screen distortion
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		// Could trigger post-process effects here
		ShakeCamera(PC, 2.0f);
	}
}

void AEyeMonster::TriggerProximityFearEffect()
{
	// Subtle effects for being too close
	if (ProximityFearSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ProximityFearSound, GetActorLocation(), 0.3f);
	}

	// Light camera shake
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		ShakeCamera(PC, 0.3f);
	}
}

void AEyeMonster::TriggerBlinkEffects()
{
	// Effects for aggressive blink reappearance
	if (BlinkEffectSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BlinkEffectSound, GetActorLocation());
	}

	// Spawn particle burst
	if (BlinkParticleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BlinkParticleEffect, GetActorLocation());
	}
}

void AEyeMonster::SpawnAggressiveEffects()
{
	// Spawn intense visual effects around the monster
	if (AggressiveAuraEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), AggressiveAuraEffect, GetActorLocation());
	}

	// Enhance existing aura
	if (AuraEffect)
	{
		AuraEffect->SetFloatParameter(TEXT("Intensity"), 3.0f);
		AuraEffect->SetColorParameter(TEXT("Color"), FLinearColor::Red);
	}
}

// Enhanced blink mechanics
void AEyeMonster::DoBlink()
{
	if (bIsBlinking)
		return;

	bIsBlinking = true;
	bIsCoveringEyes = true;

	if (CoverEyesMontage && MeshComponent)
	{
		MeshComponent->PlayAnimation(CoverEyesMontage, false);
	}

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
	// Enhanced blink teleportation
	FVector NewLocation;

	switch (CurrentState)
	{
	case EEyeMonsterState::Aggressive:
		NewLocation = FindAggressivePosition(GetPlayerLocation());
		break;
	case EEyeMonsterState::Retreating:
		NewLocation = FindEscapePosition();
		break;
	default:
		NewLocation = FindStealthyPosition();
		break;
	}

	if (NewLocation != FVector::ZeroVector)
	{
		SetActorLocation(NewLocation);
	}
	else
	{
		// Fallback to original random blink
		const FVector Origin = GetActorLocation();
		const FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, BlinkRadius);
		FVector FallbackLocation = Origin + RandomOffset;
		SetActorLocation(FallbackLocation);
	}

	bIsCoveringEyes = false;
	bIsBlinking = false;
	LastBlinkTime = GetWorld()->GetTimeSeconds();
}

void AEyeMonster::SetIlluminated(bool bOn)
{
	if (bOn)
	{
		if (!bIsCoveringEyes)
		{
			bIsCoveringEyes = true;
			if (CoverEyesMontage && MeshComponent)
			{
				MeshComponent->PlayAnimation(CoverEyesMontage, false);
			}

			// Chance to blink away when illuminated
			if (FMath::RandRange(0.0f, 1.0f) < 0.3f && CanPerformBlink())
			{
				PerformEscapeBlink();
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