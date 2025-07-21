#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "HorrorGame/Interface/Interact.h"
#include "Components/TimelineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawerCabinetActor.generated.h"

UCLASS()
class HORRORGAME_API ADrawerCabinetActor : public AInteractableActor, public IInteract
{
    GENERATED_BODY()

public:
    ADrawerCabinetActor();

protected:
    virtual void BeginPlay() override;

    virtual void Interact(AHorrorGameCharacter* Player) override;

    UPROPERTY(EditAnywhere, Category = "Drawer | Sound")
	USoundBase* OpenSound;

    UPROPERTY(EditAnywhere, Category = "Drawer | Sound")
	USoundBase* CloseSound;

private:
    void ToggleDrawer(int32 DrawerIndex);
    UFUNCTION()
    void HandleDrawerProgress(float Value);

    int32 GetClosestOverlappingDrawer(AHorrorGameCharacter* Player);

    UPROPERTY()
    USceneComponent* SceneRoot;

    UPROPERTY(EditAnywhere, Category = "Drawer")
    TArray<UStaticMeshComponent*> DrawerMeshes;

    UPROPERTY(EditAnywhere, Category = "Drawer")
    TArray<UBoxComponent*> DrawerBox;

    UPROPERTY(EditAnywhere, Category = "Drawer")
    TArray<UWidgetComponent*> DrawerWidgets;

    UPROPERTY(EditAnywhere, Category = "Drawer")
    TArray<bool> bIsOpen;

    UPROPERTY(EditAnywhere, Category = "Drawer")
    TArray<UTimelineComponent*> DrawerTimelines;

    UPROPERTY(EditAnywhere, Category = "Drawer")
    UCurveFloat* OpenCurve;

    TArray<FVector> InitialLocations;
    TArray<FVector> TargetLocations;

    int32 CurrentDrawerIndex;
};