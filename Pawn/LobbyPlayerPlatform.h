// LobbyPlayerPlatform.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyPlayerPlatform.generated.h"

UENUM(BlueprintType)
enum class EPlatformState : uint8
{
    Empty,
    Occupied,
    Ready
};

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
    bool HasPlayer() const;

    UPROPERTY(EditAnywhere, Category = "Lobby")
    TSubclassOf<AActor> CharacterClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* PlatformLight;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "State")
    bool bIsOccupied;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "State")
    EPlatformState PlatformState;

    UFUNCTION(BlueprintCallable)
    void SetPlayerReady(bool bReady);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void UpdatePlatformVisuals();

    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicMaterial;

    float PlatformAnimTime = 0.0f;


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