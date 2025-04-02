// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/AI/AICharacterBase.h"
#include "Components/WidgetComponent.h"
#include "HorrorGame/Data/Const.h"
#include "Components/BoxComponent.h"
#include "HealthBarWidget.h"
#include "HorrorGame/AI/NPC.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "Components/TextRenderComponent.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AAICharacterBase::AAICharacterBase() :WidgetComponent{ CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Value")) }, Health{ MaxHealth }
, RightFistCollisionBox{ CreateDefaultSubobject<UBoxComponent>(TEXT("RightFirtCollisionBox")) }
{
	PrimaryActorTick.bCanEverTick = true;
	AlertTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("AlertTextComponent"));
	if (AlertTextComponent)
	{
		AlertTextComponent->SetupAttachment(GetMesh(), "head");
	}

	if (WidgetComponent)
	{
		WidgetComponent->SetupAttachment(RootComponent);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetRelativeLocation(defs::HealthBar);
		static ConstructorHelpers::FClassFinder<UUserWidget>WidgetClass{ TEXT("/Game/AI/WB_HealthBar") };
		if (WidgetClass.Succeeded())
		{
			WidgetComponent->SetWidgetClass(WidgetClass.Class);
		}
	}

	if (RightFistCollisionBox)
	{
		RightFistCollisionBox->SetBoxExtent(defs::RightFistBoxSize, false);
		FAttachmentTransformRules const Rules{
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld,
			false };
		RightFistCollisionBox->AttachToComponent(GetMesh(), Rules, defs::RightFistSocketName);
		RightFistCollisionBox->SetRelativeLocation(defs::CollisionBoxLocation);
	}
}

void AAICharacterBase::BeginPlay()
{
	Super::BeginPlay();
	RightFistCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AAICharacterBase::OnAttackOverlapBegin);
	RightFistCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AAICharacterBase::OnAttackOverlapEnd);
}

void AAICharacterBase::OnAttackOverlapBegin(
	UPrimitiveComponent* const OverlapComponent,
	AActor* const OtherActor,
	UPrimitiveComponent* const OtherComponent,
	int const OtherBodyIndex,
	bool const FromSweep,
	FHitResult const& SweepResult)
{
	if (OtherActor == this || !OtherActor)
	{
		return;
	}

	if (auto const Enemy = Cast<ANPC>(OtherActor))
	{
		float DamageAmount = Enemy->GetMaxHealth() * 0.1f;
		float NewHealth = Enemy->GetHealth() - DamageAmount;
		Enemy->SetHealth(NewHealth);
	}
	else if (AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(OtherActor))
	{
		float DamageAmount = 10.0f;
		UGameplayStatics::ApplyDamage(Player, DamageAmount, GetController(), this, UDamageType::StaticClass());
		UE_LOG(LogTemp, Log, TEXT("AI hit player! Damage: %f"), DamageAmount);
	}
}

void AAICharacterBase::OnAttackOverlapEnd(
	UPrimitiveComponent* const OverlapComponent,
	AActor* const OtherActor,
	UPrimitiveComponent* const OtherComponent,
	int const OtherBodyIndex)
{
}

void AAICharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*if (auto const Widget = Cast <UHealthBarWidget>(WidgetComponent->GetUserWidgetObject()))
	{
		Widget->SetBarValuePercent(Health / MaxHealth);
	}*/
}

void AAICharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAICharacterBase::SetHealth(float const NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

	//cast to player if player health lower 0 then player will lose
	if (Cast<AHorrorGameCharacter>(this))
	{
		if (Health <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("You lose!"));
			GetWorld()->GetFirstLocalPlayerFromController()->ConsoleCommand("Quit");
		}
	}
	//cast to npc if the npc health lower 0 then player will win
	else if (Cast<ANPC>(this))
	{
		if (Health <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("You Win!"));
			GetWorld()->GetFirstLocalPlayerFromController()->ConsoleCommand("Quit");
		}
	}
}

void AAICharacterBase::AttackStart() const
{
	RightFistCollisionBox->SetCollisionProfileName("Fist");
	RightFistCollisionBox->SetNotifyRigidBodyCollision(true);
}

void AAICharacterBase::AttackEnd() const
{
	RightFistCollisionBox->SetCollisionProfileName("Fist");
	RightFistCollisionBox->SetNotifyRigidBodyCollision(false);
}

