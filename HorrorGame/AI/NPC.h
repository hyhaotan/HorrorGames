#pragma once

#include "CoreMinimal.h"
#include "HorrorGame/AI/AICharacterBase.h"
#include "Camera/CameraComponent.h"
#include "PaTrolPath.h"
#include "GameFramework/Character.h"
#include "Combat_Interface.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "NPC.generated.h"

UCLASS()
class HORRORGAME_API ANPC : public AAICharacterBase, public ICombat_Interface
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ANPC();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Lấy BehaviorTree, PatrolPath, v.v... nếu bạn cần
    UBehaviorTree* GetBehaviorTree() const;
    APaTrolPath* GetPatrolPath() const;
    UAnimMontage* GetMontage() const;

    // Combat_Interface implementation
    int MeleeAttack_Implementation() override;

    /** Custom function to toggle visibility */
    UFUNCTION(BlueprintCallable, Category = "AI")
    void ToggleInvestigationWidgetVisibility();

    /**
     * Camera component được sử dụng cho góc nhìn jump scare.
     * Thay vì dùng ACameraActor, ta gắn một UCameraComponent thẳng vào NPC.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare")
    UCameraComponent* JumpScareCamera;

    /**
     * Cờ để bật/tắt việc sử dụng camera jump scare
     * (khi NPC được SetViewTarget, ta sẽ kiểm tra cờ này để trả về góc nhìn JumpScareCamera)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare")
    bool bUseJumpScareCamera = false;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Override để cho phép NPC trả về góc nhìn từ JumpScareCamera
    virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

private:
    // Behavior Tree gán trong Editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UBehaviorTree* Tree;

    // Patrol Path gán trong Editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
    APaTrolPath* PatrolPath;

    // AnimMontage nếu bạn muốn NPC dùng
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* Montage;

    // Widget investigation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
    UUserWidget* InvestigateWidget;

    bool bIsWidgetVisible;
};
