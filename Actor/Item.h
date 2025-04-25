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
class UBoxComponent;
class UFlashLightComponent;

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
    UBoxComponent* ItemCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Mesh")
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FDataTableRowHandle ItemRowHandle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
    UItemBase* ItemData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FItemData ItemDataRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UFlashLightComponent* FlashLightComp;

    using FUseItemFunction = void (AItem::*)();
    FUseItemFunction UseItemFunction;

    UFUNCTION(BlueprintCallable, Category = "Item")
    void UseItem();

    UFUNCTION(BlueprintCallable, Category = "Item")
    void OnPickup();

    UFUNCTION(BlueprintCallable, Category = "Item")
    void OnDrop(const FVector& DropLocation);

    UFUNCTION(BlueprintCallable, Category = "Item Weight")
    void SetItemWeight(int32 Weight);

    UFUNCTION(BlueprintCallable, Category = "Item")
    void AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName);

private:
    void InitializeItemData();
    void ConfigureItemBase(const FItemData& DataRow);
    void ConfigureWidget(const FItemData& DataRow);
    void ConfigureMesh(const FItemData& DataRow);
    void BindUseFunction(const FItemData& DataRow);

    void HandleHealthMedicine();
    void HandleMolotovCocktail();
    void HandleFlashExplosive();
    void HandleFlashLight();
    FItemData* GetItemData() const;
    float CalculateHealAmount(EMedicineSize Size) const;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    bool bFlashAttached;

	bool bIsItemData;
};