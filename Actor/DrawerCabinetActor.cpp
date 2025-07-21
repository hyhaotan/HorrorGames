#include "HorrorGame/Actor/DrawerCabinetActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ADrawerCabinetActor::ADrawerCabinetActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create scene root
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    SceneRoot->SetupAttachment(RootComponent);

    const int32 Num = 4;
    DrawerMeshes.SetNum(Num);
    DrawerBox.SetNum(Num);
    DrawerWidgets.SetNum(Num);
    DrawerTimelines.SetNum(Num);
    bIsOpen.Init(false, Num);

    for (int32 i = 0; i < Num; ++i)
    {
        // Mesh
        FString MeshName = FString::Printf(TEXT("DrawerMesh%d"), i);
        DrawerMeshes[i] = CreateDefaultSubobject<UStaticMeshComponent>(*MeshName);
        DrawerMeshes[i]->SetupAttachment(SceneRoot);

        // Box
        FString BoxName = FString::Printf(TEXT("DrawerBox%d"), i);
        DrawerBox[i] = CreateDefaultSubobject<UBoxComponent>(*BoxName);
        DrawerBox[i]->SetupAttachment(DrawerMeshes[i]);
        DrawerBox[i]->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
        DrawerBox[i]->InitBoxExtent(FVector(50.f, 100.f, 12.f));
        DrawerBox[i]->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

        // Widget
        FString WidgetName = FString::Printf(TEXT("DrawerWidget%d"), i);
        DrawerWidgets[i] = CreateDefaultSubobject<UWidgetComponent>(*WidgetName);
        DrawerWidgets[i]->SetupAttachment(DrawerMeshes[i]);
        DrawerWidgets[i]->SetWidgetSpace(EWidgetSpace::Screen);
        DrawerWidgets[i]->SetDrawSize(FVector2D(100, 50));

        // Timeline
        FString TimelineName = FString::Printf(TEXT("DrawerTimeline%d"), i);
        DrawerTimelines[i] = CreateDefaultSubobject<UTimelineComponent>(*TimelineName);
    }
}

void ADrawerCabinetActor::BeginPlay()
{
    Super::BeginPlay();

    InitialLocations.Reset();
    TargetLocations.Reset();

    for (int32 i = 0; i < DrawerMeshes.Num(); ++i)
    {
        if (!DrawerMeshes[i]) continue;

        const FVector Loc = DrawerMeshes[i]->GetRelativeLocation();
        InitialLocations.Add(Loc);
        TargetLocations.Add(Loc + FVector(60.f, 0.f, 0.f));

        if (OpenCurve && DrawerTimelines[i])
        {
            FOnTimelineFloat Func;
            Func.BindUFunction(this, FName("HandleDrawerProgress"));
            DrawerTimelines[i]->AddInterpFloat(OpenCurve, Func);
            DrawerTimelines[i]->SetLooping(false);
        }
    }
}

void ADrawerCabinetActor::Interact(AHorrorGameCharacter* Player)
{
    if (!Player) return;

    Player->SetCurrentInteractItem(this);

    int32 DrawerIndex = GetClosestOverlappingDrawer(Player);
    if (DrawerIndex != -1)
    {
        ToggleDrawer(DrawerIndex);
        return;
    }

    float MinDist = FLT_MAX;
    int32 BestIndex = -1;
    const FVector PlayerLoc = Player->GetActorLocation();
    for (int32 i = 0; i < DrawerMeshes.Num(); ++i)
    {
        if (!DrawerMeshes[i]) continue;
        const float Dist = FVector::Dist(PlayerLoc, DrawerMeshes[i]->GetComponentLocation());
        if (Dist < MinDist)
        {
            MinDist = Dist;
            BestIndex = i;
        }
    }
    if (BestIndex != -1)
    {
        ToggleDrawer(BestIndex);
    }
}

void ADrawerCabinetActor::ToggleDrawer(int32 DrawerIndex)
{
    if (!DrawerMeshes.IsValidIndex(DrawerIndex) || !DrawerTimelines.IsValidIndex(DrawerIndex)) return;
    CurrentDrawerIndex = DrawerIndex;

    if (!bIsOpen[DrawerIndex]) DrawerTimelines[DrawerIndex]->PlayFromStart();
    else DrawerTimelines[DrawerIndex]->ReverseFromEnd();

    bIsOpen[DrawerIndex] = !bIsOpen[DrawerIndex];
    const bool bIsOpening = bIsOpen[DrawerIndex];
    UGameplayStatics::PlaySoundAtLocation(this, bIsOpening ? OpenSound : CloseSound, GetActorLocation());
}

void ADrawerCabinetActor::HandleDrawerProgress(float Value)
{
    if (!DrawerMeshes.IsValidIndex(CurrentDrawerIndex) ||
        !InitialLocations.IsValidIndex(CurrentDrawerIndex) ||
        !TargetLocations.IsValidIndex(CurrentDrawerIndex))
    {
        return;
    }
    const FVector NewLoc = FMath::Lerp(
        InitialLocations[CurrentDrawerIndex], TargetLocations[CurrentDrawerIndex], Value);
    DrawerMeshes[CurrentDrawerIndex]->SetRelativeLocation(NewLoc);
}

int32 ADrawerCabinetActor::GetClosestOverlappingDrawer(AHorrorGameCharacter* Player)
{
    int32 ClosestIndex = -1;
    float ClosestDist = FLT_MAX;

    for (int32 i = 0; i < DrawerBox.Num(); ++i)
    {
        if (DrawerBox[i] && DrawerBox[i]->IsOverlappingActor(Player))
        {
            float Dist = FVector::Dist(Player->GetActorLocation(), DrawerBox[i]->GetComponentLocation());
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                ClosestIndex = i;
            }
        }
    }

    return ClosestIndex;
}
