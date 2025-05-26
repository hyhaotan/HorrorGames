#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Data/ItemData.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

class UParticleSystem;
class UItemBase;
class UPostProcessComponent;
class UMaterialInstanceDynamic;
class UCameraShakeBase;
class UCurveFloat;
class UUserWidget;
class UAnimMontage;
class USoundBase;
class UTimelineComponent;
class UWidgetComponent;
class USphereComponent;
class UFlashLightComponent;
class UWidgetAnimation;
class UItemWidget;

UCLASS()
class HORRORGAME_API AItem : public AActor
{
    GENERATED_BODY()

public:
    AItem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Widget")
    UWidgetComponent* ItemWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box Item")
    USphereComponent* SphereComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Mesh")
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FDataTableRowHandle ItemRowHandle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
    UItemBase* ItemData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FItemData ItemDataRow;

    UPROPERTY()
    UItemWidget* PickupWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    TSubclassOf<UItemWidget> PickupWidgetClass;

	//============FUNCTION==================//

    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void UseItem();

    UFUNCTION(BlueprintCallable, Category = "Item")
    void OnPickup();

    UFUNCTION(BlueprintCallable, Category = "Item")
    void OnDrop(const FVector& DropLocation);

    UFUNCTION(BlueprintCallable, Category = "Item")
    void AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName);

    void InitializeItemData();

    void InitializeFrom(const AItem* Source, int32 InQuantity);

    FItemData* GetItemData() const;
    //============AVAIABLE=================//
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stack")
    bool bIsStackable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack", meta = (EditCondition = "bIsStackable"))
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack", meta = (EditCondition = "bIsStackable"))
    int32 MaxStackSize = 1;

private:

    void ConfigureItemBase(const FItemData& DataRow);
    void ConfigureMesh(const FItemData& DataRow);
    void InitializeStackProperties(const FItemData& DataRow);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    virtual void OnConstruction(const FTransform& Transform) override;

	bool bIsItemData;
};