#include "BomComponent.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Actor/Item.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "HorrorGame/Actor/GrenadeProjectile.h"
#include "HorrorGame/Actor/FireZone.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/TimelineComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/ProjectileMovementComponent.h"

UBomComponent::UBomComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	// Khởi tạo các giá trị mặc định nếu cần
	ProjectileSpeeds = 1000.f;
	FlashDuration = 1.0f;
	FlashMaxDistance = 1000.f;
	FlashViewAngleThreshold = 45.f;
	FlashTimeline = nullptr;
	FlashMaterialInstance = nullptr;
	FlashPostProcess = nullptr;
}

void UBomComponent::BeginPlay()
{
	Super::BeginPlay();
	// Khởi tạo nếu cần
}

void UBomComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Xử lý tick nếu cần
}

FVector UBomComponent::GetSpawnLocation() const
{
	const float OffsetDistance = 100.f;
	// Sử dụng vị trí của chủ sở hữu component
	return GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * OffsetDistance;
}

FVector UBomComponent::CalculateTargetLocationForBomb()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	AActor* Owner = GetOwner();
	if (Owner)
	{
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			if (AController* Controller = Pawn->GetController())
			{
				Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
			}
		}
	}

	const float TraceDistance = 10000.f;
	FVector TraceStart = ViewLocation;
	FVector TraceEnd = TraceStart + (ViewRotation.Vector() * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	if (GetWorld())
	{
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

#if WITH_EDITOR
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 2.0f);
#endif
	}

	return HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEnd;
}

float UBomComponent::ComputeInitialSpeed(float Distance, float Gravity, float AngleInRadians) const
{
	if (FMath::IsNearlyZero(FMath::Sin(2 * AngleInRadians)))
	{
		return 0.f;
	}
	return FMath::Sqrt((Distance * Gravity) / FMath::Sin(2 * AngleInRadians));
}

FVector UBomComponent::ComputeLaunchVelocity(const FVector& SpawnLocation, const FVector& TargetLocation, float InProjectileSpeed, float AngleInRadians) const
{
	FVector Delta = TargetLocation - SpawnLocation;
	float Distance = Delta.Size();
	float Gravity = (GetWorld() ? GetWorld()->GetGravityZ() * -1.f : 980.f);

	float InitialSpeed = (InProjectileSpeed > 0.f) ? InProjectileSpeed : ComputeInitialSpeed(Distance, Gravity, AngleInRadians);
	FVector LaunchDirection = Delta.GetSafeNormal();

	FVector HorizontalDirection = LaunchDirection;
	HorizontalDirection.Z = 0.f;
	HorizontalDirection = HorizontalDirection.GetSafeNormal();

	FVector Velocity = HorizontalDirection * InitialSpeed * FMath::Cos(AngleInRadians);
	Velocity.Z = InitialSpeed * FMath::Sin(AngleInRadians);
	return Velocity;
}

void UBomComponent::ThrowBomb(const FVector& TargetLocation, float InProjectileSpeed)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("ThrowBomb: GetWorld() returned nullptr."));
		return;
	}

	FVector SpawnLocation = GetSpawnLocation();
	FVector Delta = TargetLocation - SpawnLocation;
	float Distance = Delta.Size();
	if (Distance < KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Warning, TEXT("ThrowBomb: Distance is too small, aborting throw."));
		return;
	}

	constexpr float AngleInDegrees = 45.f;
	const float AngleInRadians = FMath::DegreesToRadians(AngleInDegrees);
	FVector Velocity = ComputeLaunchVelocity(SpawnLocation, TargetLocation, InProjectileSpeed, AngleInRadians);

	SpawnAndThrowBomb(Velocity);
}

void UBomComponent::SpawnAndThrowBomb(const FVector& Velocity)
{

	// Lấy owner để xác định vị trí spawn
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner of BombComponent is null!"));
		return;
	}

	// Vị trí và hướng spawn bom
	const FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.0f;
	const FRotator SpawnRotation = Owner->GetActorRotation();

	// Thiết lập thông tin spawn
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Owner->GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn bomb
	AGrenadeProjectile* SpawnedBomb = GetWorld()->SpawnActor<AGrenadeProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!SpawnedBomb)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn bomb actor!"));
		return;
	}

	// Kích hoạt mô phỏng vật lý và áp lực ném
	if (UPrimitiveComponent* BombPrimitive = Cast<UPrimitiveComponent>(SpawnedBomb->GetComponentByClass(UPrimitiveComponent::StaticClass())))
	{
		BombPrimitive->SetSimulatePhysics(true);
		BombPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		BombPrimitive->AddImpulse(Velocity, NAME_None, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("The spawned bomb does not have a primitive component to apply impulse on."));
	}
}

void UBomComponent::ActivateAndThrowBomb(const FVector& TargetLocation, float InProjectileSpeed, bool bIsFlashBomb)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateAndThrowBomb: GetWorld() is nullptr."));
		return;
	}

	if (bIsFlashBomb)
	{
		UE_LOG(LogTemp, Log, TEXT("ActivateAndThrowBomb: Flash bomb activated."));
		ThrowBomb(TargetLocation, InProjectileSpeed);
		GetWorld()->GetTimerManager().SetTimer(BombActivationTime, this, &UBomComponent::ExplodeFlash,3.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ActivateAndThrowBomb: Molotov or other bomb activated."));
		if (IgniteEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), IgniteEffect, GetOwner()->GetActorLocation());
		}
		ThrowBomb(TargetLocation, InProjectileSpeed);
		GetWorld()->GetTimerManager().SetTimer(BombActivationTime, this, &UBomComponent::MolotovCocktail, 3.0f, false);
	}
}

void UBomComponent::HandleFlashExplosive()
{
	FVector TargetLocation = CalculateTargetLocationForBomb();
	ActivateAndThrowBomb(TargetLocation, ProjectileSpeeds, true);
}

void UBomComponent::HandleMolotovCocktail()
{
	FVector TargetLocation = CalculateTargetLocationForBomb();
	ActivateAndThrowBomb(TargetLocation, ProjectileSpeeds, false);
}

void UBomComponent::ExplodeFlash()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}

	const FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FVector BombDir = (GetOwner()->GetActorLocation() - CameraLoc).GetSafeNormal();
	const FVector CameraForward = PC->PlayerCameraManager->GetActorForwardVector();

	float Dot = FMath::Clamp(FVector::DotProduct(CameraForward, BombDir), -1.0f, 1.0f);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));
	const float Distance = FVector::Dist(CameraLoc, GetOwner()->GetActorLocation());

	const bool bShouldFlash = (Distance <= FlashMaxDistance) && (AngleDeg <= FlashViewAngleThreshold);

	if (bShouldFlash)
	{
		TriggerFlashWidget();
		TriggerFlashPostProcess();
		SpawnParticleEffect();
		TriggerCameraShake(PC);
		PlayFlashReactionMontage(PC->GetPawn());
	}

	PlaySoundEffect(GetOwner()->GetActorLocation());
	GetOwner()->Destroy();
}

bool UBomComponent::IsPlayerLookingAtMe() const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return false;
	}

	const FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FVector BombDir = (GetOwner()->GetActorLocation() - CameraLoc).GetSafeNormal();
	const FVector CameraForward = PC->PlayerCameraManager->GetActorForwardVector();

	const float Dot = FMath::Clamp(FVector::DotProduct(CameraForward, BombDir), -1.0f, 1.0f);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

	return AngleDeg <= FlashViewAngleThreshold;
}

void UBomComponent::TriggerFlashWidget()
{
	if (!FlashWidgetClass)
	{
		return;
	}

	UUserWidget* FlashWidget = CreateWidget<UUserWidget>(GetWorld(), FlashWidgetClass);
	if (!FlashWidget)
	{
		return;
	}

	FlashWidget->AddToViewport();

	FTimerHandle WidgetTimer;
	GetWorld()->GetTimerManager().SetTimer(WidgetTimer, [FlashWidget]()
		{
			if (FlashWidget)
			{
				FlashWidget->RemoveFromParent();
			}
		}, FlashDuration, false);
}

void UBomComponent::TriggerFlashPostProcess()
{
	TArray<AActor*> OverlappedActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), OverlappedActors);

	for (AActor* Actor : OverlappedActors)
	{
		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character || (FVector::Dist(GetOwner()->GetActorLocation(), Character->GetActorLocation()) > FlashMaxDistance))
		{
			continue;
		}

		FVector DirToFlash = (GetOwner()->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
		FVector PlayerViewDir = Character->GetControlRotation().Vector();
		const float Dot = FMath::Clamp(FVector::DotProduct(DirToFlash, PlayerViewDir), -1.0f, 1.0f);
		const float ViewAngle = FMath::RadiansToDegrees(FMath::Acos(Dot));

		if (ViewAngle <= 90.0f)
		{
			if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
			{
				TriggerCameraShake(PC);
				PlaySoundEffectForController(PC);
				ApplyFlashEffectToScreen(PC, 5.0f);
			}
		}
	}

	SpawnParticleEffect();
	PlaySoundEffect(GetOwner()->GetActorLocation());
}

void UBomComponent::ApplyFlashEffectToScreen(APlayerController* PlayerController, float Duration)
{
	if (!PlayerController)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn());
	if (!Character)
	{
		return;
	}

	UPostProcessComponent* PostProcessComp = Character->FindComponentByClass<UPostProcessComponent>();
	if (!PostProcessComp)
	{
		PostProcessComp = NewObject<UPostProcessComponent>(Character);
		if (PostProcessComp)
		{
			PostProcessComp->RegisterComponent();
			PostProcessComp->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	if (PostProcessComp)
	{
		PostProcessComp->Settings.bOverride_BloomIntensity = true;
		PostProcessComp->Settings.BloomIntensity = 100.0f;

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PostProcessComp]()
			{
				if (PostProcessComp)
				{
					PostProcessComp->Settings.bOverride_BloomIntensity = false;
				}
			}, Duration, false);
	}
}

void UBomComponent::UpdateFlashEffect(float Value)
{
	if (FlashMaterialInstance)
	{
		FlashMaterialInstance->SetScalarParameterValue(FName("FlashIntensity"), Value);
	}
}

void UBomComponent::OnFlashTimelineFinished()
{
	if (FlashPostProcess)
	{
		FlashPostProcess->BlendWeight = 0.0f;
		FlashPostProcess->Settings.WeightedBlendables.Array.Empty();
	}
}

void UBomComponent::FlashCurves()
{
	if (!FlashCurve)
	{
		return;
	}

	FlashTimeline = NewObject<UTimelineComponent>(this, FName("FlashTimeline"));
	if (!FlashTimeline)
	{
		return;
	}

	FlashTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	FlashTimeline->SetPropertySetObject(this);
	FlashTimeline->SetLooping(false);

	FOnTimelineFloat TimelineCallback;
	TimelineCallback.BindUFunction(this, FName("UpdateFlashEffect"));
	FlashTimeline->AddInterpFloat(FlashCurve, TimelineCallback);

	FOnTimelineEvent TimelineFinishedCallback;
	TimelineFinishedCallback.BindUFunction(this, FName("OnFlashTimelineFinished"));
	FlashTimeline->SetTimelineFinishedFunc(TimelineFinishedCallback);

	FlashTimeline->RegisterComponent();
}

void UBomComponent::PlaySoundEffect(const FVector& Location)
{
	if (FlashSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FlashSound, Location);
	}
}

void UBomComponent::PlaySoundEffectForController(APlayerController* PC)
{
	if (FlashSound && PC)
	{
		PC->ClientPlaySound(FlashSound);
	}
}

void UBomComponent::SpawnParticleEffect()
{
	if (ExplosionEffect)
	{
		UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetOwner()->GetActorLocation());
		if (ParticleComp)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([ParticleComp]()
				{
					if (ParticleComp)
					{
						ParticleComp->DestroyComponent();
					}
				}), 1.0f, false);
		}
	}
}

void UBomComponent::TriggerCameraShake(APlayerController* PC)
{
	//if (FlashCameraShake && PC)
	//{
	//	PC->ClientStartCameraShake(FlashCameraShake);
	//}
}

void UBomComponent::PlayFlashReactionMontage(APawn* Pawn)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(Pawn);
	if (PlayerCharacter && FlashReactionMontage)
	{
		if (UAnimInstance* AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(FlashReactionMontage, 1.0f);
		}
	}
}

void UBomComponent::MolotovCocktail()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: GetWorld() returned nullptr."));
		return;
	}

	FTimerHandle DelayTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayTimerHandle, [this]()
		{
			if (!GetWorld()) return;

			if (MolotovCocktailSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, MolotovCocktailSound, GetOwner()->GetActorLocation());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: MolotovCocktailSound is nullptr."));
			}

			if (FireEffect)
			{
				UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireEffect, GetOwner()->GetActorLocation());
				if (NiagaraComp)
				{
					TWeakObjectPtr<UNiagaraComponent> WeakNiagaraComp = NiagaraComp;
					FTimerHandle NiagaraTimerHandle;
					GetWorld()->GetTimerManager().SetTimer(NiagaraTimerHandle, [WeakNiagaraComp]()
						{
							if (WeakNiagaraComp.IsValid())
							{
								WeakNiagaraComp->DestroyComponent();
							}
						}, 5.0f, false);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: FireEffect is nullptr."));
			}

			if (FireZoneClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
				{
					SpawnParams.Instigator = PawnOwner;
				}
				AFireZone* FireZone = GetWorld()->SpawnActor<AFireZone>(FireZoneClass, GetOwner()->GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
				if (FireZone)
				{
					FireZone->SetLifeSpan(5.0f);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: FireZoneClass is nullptr."));
			}

			UE_LOG(LogTemp, Log, TEXT("Molotov Cocktail: Fire activated after delay."));
			GetOwner()->Destroy();

		}, 3.0f, false);
}
