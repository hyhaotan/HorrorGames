// DoorComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DoorComponent.generated.h"

class UCurveFloat;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORGAME_API UDoorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDoorComponent();

protected:
	virtual void BeginPlay() override;

	/** Timeline để animate cửa */
	UPROPERTY(Transient)
	class UTimelineComponent* DoorTimeline;

	/** Curve driving the open animation (0->1) */
	UPROPERTY(EditAnywhere, Category = "Door|Animation")
	UCurveFloat* DoorOpenCurve;

	/** Pivot để xoay cửa */
	UPROPERTY(EditAnywhere, Category = "Door")
	USceneComponent* DoorPivot;

	/** Mesh của cánh cửa */
	UPROPERTY(EditAnywhere, Category = "Door")
	UStaticMeshComponent* DoorMesh;

	/** Góc đóng & mở */
	UPROPERTY(EditAnywhere, Category = "Door|Animation")
	FRotator ClosedRotation;
	UPROPERTY(EditAnywhere, Category = "Door|Animation")
	FRotator OpenRotation;

	bool bHasOpened;

	/** Xử lý mỗi frame của Timeline */
	UFUNCTION()
	void HandleDoorProgress(float Value);

public:
	/** Gọi để mở cửa (nếu chưa mở) */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor();
};
