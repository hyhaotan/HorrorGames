#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FlashLightComponent.generated.h"

class AEyeMonster;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API UFlashLightComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UFlashLightComponent();

    /** Khoảng trace để chiếu tia */
    UPROPERTY(EditAnywhere, Category = "Flashlight")
    float TraceDistance = 2000.f;

    /** Ngưỡng dot trước khi monster chớp */
    UPROPERTY(EditAnywhere, Category = "Flashlight")
    float LightDotThreshold = 0.8f;

    /** SpotLight con */
    UPROPERTY(VisibleAnywhere, Category = "Flashlight")
    class USpotLightComponent* SpotLight;

    /** Bật/tắt đèn pin */
    void ToggleFlashlight();

    UPROPERTY(EditAnywhere, Category = "FlashLight | Sound")
	USoundBase* FlashLightOnSound;

	UPROPERTY(EditAnywhere, Category = "FlashLight | Sound")
    USoundBase* FlashLightOffSound;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool bIsOn;
    /** Xử lý trace và làm hiệu ứng khi bật */
    void HandleFlashLight();

    TWeakObjectPtr<AEyeMonster> LastIlluminatedMonster;
};
