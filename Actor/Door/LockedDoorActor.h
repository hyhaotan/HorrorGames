#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/Actor/Door/DoorRootActor.h"
#include "LockedDoorActor.generated.h"

class UStaticMeshComponent;
class AHorrorGameCharacter;

UCLASS()
class HORRORGAME_API ALockedDoorActor : public ADoorRootActor
{
    GENERATED_BODY()

public:
    ALockedDoorActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Mesh hiển thị ổ khóa */
    UPROPERTY(EditAnywhere, Category = "Door")
    UStaticMeshComponent* LockMesh;

    /** ID chìa cần thiết để mở cửa */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
    FName RequiredKeyID;

    /** Trạng thái khóa / mở */
    UPROPERTY(ReplicatedUsing = OnRep_IsLocked)
    bool bIsLocked;

    /** OnRep callback cho trạng thái khóa */
    UFUNCTION()
    void OnRep_IsLocked();

    /** Unlock door (gỡ khóa, xóa chìa, show thành công) */
    void UnlockDoor(AHorrorGameCharacter* Player);

    /** Kiểm tra điều kiện mở cửa riêng cho LockedDoorActor */
    virtual bool CanOpenDoor_Implementation(AHorrorGameCharacter* Player) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};