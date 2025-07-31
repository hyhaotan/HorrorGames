#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Data/ItemData.h"
#include "HorrorGame/Actor/InteractableActor.h"
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
class HORRORGAME_API AItem : public AInteractableActor
{
    GENERATED_BODY()

public:
    AItem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:

    UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Item Data")
    FDataTableRowHandle ItemRowHandle;

    UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadOnly, Category = "Item Data")
    UItemBase* ItemData;

    UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadOnly, Category = "Item Data")
	FItemData ItemDataRow;

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

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnPickedUp();

    FItemData* GetItemData() const;
    //============AVAIABLE=================//

private:

    void ConfigureItemBase(const FItemData& DataRow);
    void ConfigureMesh(const FItemData& DataRow);

	bool bIsItemData;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};