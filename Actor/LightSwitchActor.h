#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "HorrorGame/Interface/Interact.h"
#include "LightSwitchActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class USoundBase;

UCLASS()
class HORRORGAME_API ALightSwitchActor : public AInteractableActor, public IInteract
{
    GENERATED_BODY()

public:
    ALightSwitchActor();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Khi player bấm “Interact” — chỉ gọi server-RPC trên Character
    virtual void Interact(class AHorrorGameCharacter* Interactor) override;

    // Multicast để broadcast tới tất cả client
    UFUNCTION(NetMulticast, Reliable)
    void MulticastToggleLightSwitch();

protected:
    // Component & state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Switch", meta = (AllowPrivateAccess))
    UStaticMeshComponent* LightMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Switch", meta = (AllowPrivateAccess))
    UPointLightComponent* PointLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Switch", meta = (AllowPrivateAccess))
    UStaticMeshComponent* SwitchMesh;

    // Trạng thái và OnRep để sync cho những client join sau
    UPROPERTY(ReplicatedUsing = OnRep_IsOn, VisibleAnywhere, BlueprintReadOnly, Category = "Light Switch", meta = (AllowPrivateAccess))
    bool bIsOn = false;

    UFUNCTION()
    void OnRep_IsOn();

    // Logic bật/tắt thật sự
    void DoToggle();

    // Âm thanh
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Switch|Sound", meta = (AllowPrivateAccess))
    USoundBase* SwitchOnSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light Switch|Sound", meta = (AllowPrivateAccess))
    USoundBase* SwitchOffSound;
};
