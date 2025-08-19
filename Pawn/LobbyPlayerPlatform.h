// LobbyPlayerPlatform.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyPlayerPlatform.generated.h"

class UStaticMeshComponent;
class UArrowComponent;

UCLASS()
class HORRORGAME_API ALobbyPlayerPlatform : public AActor
{
    GENERATED_BODY()

public:
    ALobbyPlayerPlatform();

    UFUNCTION(BlueprintCallable)
    void SpawnCharacter(class APlayerController* Player);

    UFUNCTION(BlueprintCallable)
    void Clear();

    UFUNCTION(BlueprintCallable)
    bool HasPlayer() const { return PC != nullptr; }

    UPROPERTY(EditAnywhere, Category = "Lobby")
    TSubclassOf<AActor> CharacterClass;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
    UArrowComponent* PlayerSpawnTransform;

    UPROPERTY()
    APlayerController* PC;

    UPROPERTY()
    AActor* CurrentCharacter;
};