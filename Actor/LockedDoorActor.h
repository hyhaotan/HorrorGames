// LockedDoorActor.h

#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/InteractableActor.h"
#include "HorrorGame/Interface/Interact.h"
#include "Components/TimelineComponent.h"
#include "LockedDoorActor.generated.h"

class UStaticMeshComponent;
class UCurveFloat;
class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API ALockedDoorActor : public AInteractableActor, public IInteract
{
    GENERATED_BODY()

public:
    ALockedDoorActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Pivot cho bản lề cửa */
    UPROPERTY(VisibleAnywhere, Category = "Door")
    USceneComponent* DoorPivot;

    /** Mesh của cánh cửa */
    UPROPERTY(EditAnywhere, Category = "Door")
    UStaticMeshComponent* DoorMesh;

    /** Mesh hiển thị ổ khóa */
    UPROPERTY(EditAnywhere, Category = "Door")
    UStaticMeshComponent* LockMesh;

    /** ID chìa cần thiết để mở cửa */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
    FName RequiredKeyID;

    /** Timeline để animate cửa */
    UPROPERTY()
    UTimelineComponent* DoorTimeline;

    /** Curve driving the open animation (0->1) */
    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    UCurveFloat* DoorOpenCurve;

    /** Góc đóng & mở cửa */
    FRotator ClosedRotation;
    FRotator OpenRotation;

    /** Xử lý mỗi frame của Timeline */
    UFUNCTION()
    void HandleDoorProgress(float Value);

    /** Play animation mở cửa */
    void PlayOpenDoorAnim();

private:
    /** Logic khi người chơi tương tác */
    virtual void Interact(AHorrorGameCharacter* Player) override;

    /** Unlock door (gỡ khóa, xóa chìa, show thành công) */
    void UnlockDoor(AHorrorGameCharacter* Player);

    /** Trạng thái khóa / mở */
    bool bIsLocked;
    bool bHasOpened;  // true khi đã play animation mở cửa
};
