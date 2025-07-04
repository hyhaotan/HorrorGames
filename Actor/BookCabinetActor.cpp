#include "BookCabinetActor.h"
#include "HorrorGame/Widget/TextScreenWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ABookCabinetActor::ABookCabinetActor()
{
    PrimaryActorTick.bCanEverTick = true;

    CabinetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabinetMesh"));
    RootComponent = CabinetMesh;

    OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
    OverlapBox->SetupAttachment(RootComponent);
    OverlapBox->SetBoxExtent(FVector(100.f));
    OverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapBox->SetCollisionObjectType(ECC_WorldDynamic);
    OverlapBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    OverlapBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ABookCabinetActor::OnOverlapBegin);
}

void ABookCabinetActor::BeginPlay()
{
    Super::BeginPlay();
}

void ABookCabinetActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABookCabinetActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (OtherActor == PlayerPawn)
        {
            OverlapBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            SpawnBooks();
            SetTime();
        }
    }
}

void ABookCabinetActor::SpawnBooks()
{
    FVector Forward = GetActorForwardVector();
    FVector SpawnLocation = GetActorLocation() + Forward * 200.f + FVector(2000.f, 1250.f, 200.f);
    FRotator SpawnRotation = GetActorRotation();

    DrawDebugSphere(GetWorld(), SpawnLocation, 25.f, 12, FColor::Green, false, 5.f);

    AActor* Spawned = GetWorld()->SpawnActor<AActor>(BookClass, SpawnLocation, SpawnRotation);
    if (Spawned)
    {
        UE_LOG(LogTemp, Warning, TEXT("Spawned book at %s"), *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnActor returned nullptr"));
    }
}

void ABookCabinetActor::PlaySound1()
{
    if (Sound1)
    {
        UGameplayStatics::PlaySound2D(this, Sound1);
    }
}

void ABookCabinetActor::SetTime()
{
    FTimerHandle WidgetTimerHandle;
    FTimerHandle SoundTimerHandle;

    if (TextScreenClass)
    {
        UTextScreenWidget* TextScreenWidget = CreateWidget<UTextScreenWidget>(GetWorld(), TextScreenClass);
        if (TextScreenWidget)
        {
            FText TextToDisplay = TextScreenWidget->GetDisplayText();
            TextScreenWidget->SetShowAnimation();
            TextScreenWidget->SetTextBlockText(TextToDisplay);
            GetWorld()->GetTimerManager().SetTimer(
                WidgetTimerHandle,
                FTimerDelegate::CreateLambda([TextScreenWidget]() {
                    TextScreenWidget->AddToViewport(100);
                    }),
                1.0f,
                false
            );
        }
    }

    GetWorld()->GetTimerManager().SetTimer(SoundTimerHandle, this, &ABookCabinetActor::PlaySound1, 1.0f, false);
}