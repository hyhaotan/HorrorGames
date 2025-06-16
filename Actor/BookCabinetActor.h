#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BookCabinetActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTextScreenWidget;

UCLASS()
class HORRORGAME_API ABookCabinetActor : public AActor
{
    GENERATED_BODY()

public:
    ABookCabinetActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Overlap event
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

private:
    // Components
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* CabinetMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* OverlapBox;

    // Book to spawn
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TSubclassOf<AActor> BookClass;

    // Widget
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UTextScreenWidget> TextScreenClass;

    // Sound
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    USoundBase* Sound1;

    // Helpers
    void SpawnBooks();
    void PlaySound1();
    void SetTime();
};