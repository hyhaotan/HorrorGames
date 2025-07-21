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

    /** Server RPC để xử lý Interact */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInteract(AHorrorGameCharacter* Player);
    bool ServerInteract_Validate(AHorrorGameCharacter* Player);
    void ServerInteract_Implementation(AHorrorGameCharacter* Player);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Pivot cho bản lề cửa */
    UPROPERTY(EditAnywhere, Category = "Door")
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

    /** OnRep callbacks */
    UFUNCTION()
    void OnRep_IsLocked();

    UFUNCTION()
    void OnRep_HasOpened();

private:
    /** Logic khi người chơi tương tác */
    virtual void Interact(AHorrorGameCharacter* Player) override;

    /** Multicast RPC để play animation mở cửa cho tất cả client */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayOpenDoor();

    /** Unlock door (gỡ khóa, xóa chìa, show thành công) */
    void UnlockDoor(AHorrorGameCharacter* Player);

    /** Trạng thái khóa / mở */
    UPROPERTY(ReplicatedUsing = OnRep_IsLocked)
    bool bIsLocked;

    UPROPERTY(ReplicatedUsing = OnRep_HasOpened)
    bool bHasOpened;  // true khi đã play animation mở cửa

    // Override để replicate biến
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};