#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorGame/Data/ItemData.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

UCLASS()
class HORRORGAME_API AItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Widget")
	class UWidgetComponent* ItemWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box Item")
	class UBoxComponent* ItemCollission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Mesh")
	class UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FDataTableRowHandle ItemRowHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	class UItemBase* ItemData;

	// Các hàm tương tác
	UFUNCTION(BlueprintCallable, Category = "Item")
	void OnPickup();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void OnDrop(const FVector& DropLocation);

	UFUNCTION(BlueprintCallable,Category = "Item Weight")
	void SetItemWeight(int32 Weight);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);


	//------------------------------------------------AVAIABLE--------------------------------------------------------//

private:
	// Hàm khởi tạo dữ liệu cho item dựa vào DataTable
	void InitializeItemData();
};
