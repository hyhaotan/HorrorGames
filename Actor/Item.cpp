// Item.cpp

#include "HorrorGame/Actor/Item.h"
#include "HorrorGame/Widget/ItemWidget.h"
#include "HorrorGame/Item/ItemBase.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Actor/FireZone.h"

#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "Components/PostProcessComponent.h"
#include "Components/TimelineComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Tạo ItemMesh, đặt làm RootComponent và bật simulate physics
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;
    ItemMesh->SetSimulatePhysics(false);

    // Tạo ItemCollision và attach vào ItemMesh
    ItemCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemCollision"));
    ItemCollision->SetupAttachment(ItemMesh);

    // Tạo ItemWidget, attach vào ItemCollision và cấu hình
    ItemWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemWidget"));
    ItemWidget->SetupAttachment(ItemCollision);
    ItemWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemWidget->SetVisibility(false);

    // Gán các sự kiện overlap cho ItemCollision
    ItemCollision->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
    ItemCollision->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);

    // Khởi tạo loại item mặc định là Normal
    ItemCategory = EItemCategory::Normal;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Game/StarterContent/Materials/M_Basic_Wall.M_Basic_Wall"));
    if (MaterialFinder.Succeeded())
    {
        FlashPostProcessMaterial = MaterialFinder.Object;
    }
}

void AItem::BeginPlay()
{
    Super::BeginPlay();
    InitializeItemData();
    FlashCurves();
}

void AItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AItem::OnPickup()
{
    // Ẩn vật phẩm, vô hiệu collision và tắt simulate physics nếu cần
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    if (ItemMesh && ItemMesh->IsSimulatingPhysics())
    {
        ItemMesh->SetSimulatePhysics(false);
    }
}

void AItem::OnDrop(const FVector& DropLocation)
{
    // Hiển thị lại vật phẩm, bật collision, bật simulate physics và đặt vị trí
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    if (ItemMesh)
    {
        ItemMesh->SetSimulatePhysics(true);
        ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    SetActorLocation(DropLocation);
}

void AItem::SetItemWeight(int32 Weight)
{
    if (ItemMesh)
    {
        ItemMesh->SetMassOverrideInKg(NAME_None, Weight, true);
    }
}

void AItem::AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName)
{
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterMesh is null. Cannot attach item."));
        return;
    }

    // Hiển thị vật phẩm, vô hiệu collision và tắt simulate physics nếu đang bật
    SetActorHiddenInGame(false);
    SetActorEnableCollision(false);
    if (ItemMesh && ItemMesh->IsSimulatingPhysics())
    {
        ItemMesh->SetSimulatePhysics(false);
    }

    // Attach vật phẩm vào mesh của nhân vật theo socket chỉ định
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToComponent(CharacterMesh, AttachRules, SocketName);

    // Cố gắng set Owner cho item thông qua CharacterMesh->GetOwner()
    if (AHorrorGameCharacter* OwnerCharacter = Cast<AHorrorGameCharacter>(CharacterMesh->GetOwner()))
    {
        SetOwner(OwnerCharacter);
        UE_LOG(LogTemp, Log, TEXT("Owner set to: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to set Owner. CharacterMesh->GetOwner() returned null or invalid type."));
    }
}


void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Khi có actor khác va chạm, hiển thị widget chỉ mục
    if (OtherActor && OtherActor != this && ItemWidget)
    {
        ItemWidget->SetVisibility(true);
    }
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    // Khi actor rời vùng collision, ẩn widget chỉ mục
    if (OtherActor && OtherActor != this && ItemWidget)
    {
        ItemWidget->SetVisibility(false);
    }
}

void AItem::InitializeItemData()
{
    if (ItemRowHandle.DataTable && !ItemRowHandle.RowName.IsNone())
    {
        static const FString ContextString(TEXT("Item Data Context"));
        FItemData* DataRow = ItemRowHandle.DataTable->FindRow<FItemData>(ItemRowHandle.RowName, ContextString, true);
        if (DataRow)
        {
            // Tạo một UItemBase mới và khởi tạo từ dữ liệu DataTable
            UItemBase* MyItem = NewObject<UItemBase>(this, UItemBase::StaticClass());
            MyItem->InitializeFromItemData(*DataRow);
            ItemData = MyItem;

            // Cập nhật giao diện widget nếu có
            if (ItemWidget)
            {
                if (UItemWidget* MyWidget = Cast<UItemWidget>(ItemWidget->GetUserWidgetObject()))
                {
                    MyWidget->SetItemData(MyItem->ItemTextData);
                }
            }

            // Gán Static Mesh nếu có
            if (ItemMesh && DataRow->Item3DMeshData.StaticMesh)
            {
                ItemMesh->SetStaticMesh(DataRow->Item3DMeshData.StaticMesh);
                ItemMesh->SetVisibility(true);
            }
            else if (ItemMesh)
            {
                ItemMesh->SetVisibility(false);
            }
        }
    }
}

void AItem::HandleHealthMedicine()
{
    if (AHorrorGameCharacter* Character = Cast<AHorrorGameCharacter>(GetOwner()))
    {
        UE_LOG(LogTemp, Log, TEXT("Using Health Medicine: Increasing health by 25"));
        Character->IncreaseHealth(25.f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Health Medicine usage failed: Owner is not a valid character."));
    }
}

void AItem::HandleStaminaMedicine()
{
    if (AHorrorGameCharacter* Character = Cast<AHorrorGameCharacter>(GetOwner()))
    {
        UE_LOG(LogTemp, Log, TEXT("Using Stamina Medicine: Increasing stamina by 20"));
        Character->IncreaseStamina(20.f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Stamina Medicine usage failed: Owner is not a valid character."));
    }
}

void AItem::ExplodeFlash()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    // Lấy vị trí camera và tính hướng từ camera tới bomb
    const FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
    const FVector BombDir = (GetActorLocation() - CameraLoc).GetSafeNormal();
    const FVector CameraForward = PC->PlayerCameraManager->GetActorForwardVector();

    // Tính góc giữa hướng camera và hướng bomb
    float Dot = FMath::Clamp(FVector::DotProduct(CameraForward, BombDir), -1.0f, 1.0f);
    const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));
    const float Distance = FVector::Dist(CameraLoc, GetActorLocation());

    // Kiểm tra điều kiện kích hoạt flash
    const bool bShouldFlash = (Distance <= FlashMaxDistance) && (AngleDeg <= FlashViewAngleThreshold);

    if (bShouldFlash)
    {
        TriggerFlashWidget();
        TriggerFlashPostProcess();
        SpawnParticleEffect();
        TriggerCameraShake(PC);
        PlayFlashReactionMontage(PC->GetPawn());
    }

    // Phát âm thanh flash explosion
    PlaySoundEffect(GetActorLocation());

    // Hủy actor bomb sau khi nổ
    Destroy();
}

bool AItem::IsPlayerLookingAtMe() const
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
    {
        return false;
    }

    const FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
    const FVector BombDir = (GetActorLocation() - CameraLoc).GetSafeNormal();
    const FVector CameraForward = PC->PlayerCameraManager->GetActorForwardVector();

    const float Dot = FMath::Clamp(FVector::DotProduct(CameraForward, BombDir), -1.0f, 1.0f);
    const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

    return AngleDeg <= FlashViewAngleThreshold;
}

void AItem::TriggerFlashWidget()
{
    if (!FlashWidgetClass)
    {
        return;
    }

    UUserWidget* FlashWidget = CreateWidget<UUserWidget>(GetWorld(), FlashWidgetClass);
    if (!FlashWidget)
    {
        return;
    }

    FlashWidget->AddToViewport();

    // Sau FlashDuration, loại widget khỏi viewport
    FTimerHandle WidgetTimer;
    GetWorld()->GetTimerManager().SetTimer(WidgetTimer, [FlashWidget]()
        {
            if (FlashWidget)
            {
                FlashWidget->RemoveFromParent();
            }
        }, FlashDuration, false);
}

void AItem::TriggerFlashPostProcess()
{
    // Lấy danh sách tất cả các nhân vật có thể bị ảnh hưởng
    TArray<AActor*> OverlappedActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), OverlappedActors);

    for (AActor* Actor : OverlappedActors)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (!Character || (FVector::Dist(GetActorLocation(), Character->GetActorLocation()) > FlashRadius))
        {
            continue;
        }

        // Tính góc giữa hướng nhìn của nhân vật và hướng đến flashbang
        FVector DirToFlash = (GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
        FVector PlayerViewDir = Character->GetControlRotation().Vector();
        const float Dot = FMath::Clamp(FVector::DotProduct(DirToFlash, PlayerViewDir), -1.0f, 1.0f);
        const float ViewAngle = FMath::RadiansToDegrees(FMath::Acos(Dot));

        // Nếu nhân vật đang nhìn về phía flashbang, áp dụng hiệu ứng
        if (ViewAngle <= 90.0f)
        {
            APlayerController* PC = Cast<APlayerController>(Character->GetController());
            if (PC)
            {
                TriggerCameraShake(PC);
                PlaySoundEffectForController(PC);
                ApplyFlashEffectToScreen(PC, 5.0f);  // 5 giây mặc định, có thể tùy chỉnh
            }
        }
    }

    // Thực hiện particle và sound cho hiệu ứng flash (nếu chưa thực hiện từ bên trên)
    SpawnParticleEffect();
    PlaySoundEffect(GetActorLocation());
}

void AItem::ApplyFlashEffectToScreen(APlayerController* PlayerController, float Duration)
{
    if (!PlayerController)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn());
    if (!Character)
    {
        return;
    }

    // Tìm kiếm hoặc tạo mới PostProcessComponent
    UPostProcessComponent* PostProcessComp = Character->FindComponentByClass<UPostProcessComponent>();
    if (!PostProcessComp)
    {
        PostProcessComp = NewObject<UPostProcessComponent>(Character);
        if (PostProcessComp)
        {
            PostProcessComp->RegisterComponent();
            PostProcessComp->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        }
    }

    if (PostProcessComp)
    {
        // Kích hoạt hiệu ứng bloom để mô phỏng flash
        PostProcessComp->Settings.bOverride_BloomIntensity = true;
        PostProcessComp->Settings.BloomIntensity = 100.0f;

        // Reset thiết lập sau khi Duration hết hạn
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PostProcessComp]()
            {
                if (PostProcessComp)
                {
                    PostProcessComp->Settings.bOverride_BloomIntensity = false;
                }
            }, Duration, false);
    }
}

void AItem::UpdateFlashEffect(float Value)
{
    if (FlashMaterialInstance)
    {
        FlashMaterialInstance->SetScalarParameterValue(FName("FlashIntensity"), Value);
    }
}

void AItem::OnFlashTimelineFinished()
{
    if (FlashPostProcess)
    {
        FlashPostProcess->BlendWeight = 0.0f;
        FlashPostProcess->Settings.WeightedBlendables.Array.Empty();
    }
}

void AItem::FlashCurves()
{
    if (!FlashCurve)
    {
        return;
    }

    FlashTimeline = NewObject<UTimelineComponent>(this, FName("FlashTimeline"));
    if (!FlashTimeline)
    {
        return;
    }

    FlashTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
    BlueprintCreatedComponents.Add(FlashTimeline);
    FlashTimeline->SetPropertySetObject(this);
    FlashTimeline->SetLooping(false);

    // Bind timeline callbacks
    FOnTimelineFloat TimelineCallback;
    TimelineCallback.BindUFunction(this, FName("UpdateFlashEffect"));
    FlashTimeline->AddInterpFloat(FlashCurve, TimelineCallback);

    FOnTimelineEvent TimelineFinishedCallback;
    TimelineFinishedCallback.BindUFunction(this, FName("OnFlashTimelineFinished"));
    FlashTimeline->SetTimelineFinishedFunc(TimelineFinishedCallback);

    FlashTimeline->RegisterComponent();
}


void AItem::PlaySoundEffect(const FVector& Location)
{
    if (FlashSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FlashSound, Location);
    }
}

void AItem::PlaySoundEffectForController(APlayerController* PC)
{
    if (FlashSound && PC)
    {
        PC->ClientPlaySound(FlashSound);
    }
}

void AItem::SpawnParticleEffect()
{
    if (ExplosionEffect)
    {
        UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
        if (ParticleComp)
        {
            // Sử dụng Timer để hủy component sau 1 giây
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([ParticleComp]()
                {
                    if (ParticleComp)
                    {
                        ParticleComp->DestroyComponent();
                    }
                }), 1.0f, false);
        }
    }
}


void AItem::TriggerCameraShake(APlayerController* PC)
{
    if (FlashCameraShake && PC)
    {
        PC->ClientStartCameraShake(FlashCameraShake);
    }
}

void AItem::PlayFlashReactionMontage(APawn* Pawn)
{
    ACharacter* PlayerCharacter = Cast<ACharacter>(Pawn);
    if (PlayerCharacter && FlashReactionMontage)
    {
        UAnimInstance* AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(FlashReactionMontage, 1.0f);
        }
    }
}

void AItem::MolotovCocktail()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: GetWorld() returned nullptr."));
        return;
    }

    // Thiết lập timer delay 3 giây để kích hoạt hiệu ứng "cháy"
    FTimerHandle DelayTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(DelayTimerHandle, [this]()
        {
            if (!GetWorld())
            {
                return;
            }

            // Phát âm thanh kích hoạt "cháy" tại vị trí bomb
            if (MolotovCocktailSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, MolotovCocktailSound, GetActorLocation());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: MolotovCocktailSound is nullptr."));
            }

            // Spawn hiệu ứng "cháy" sử dụng Niagara effect
            if (FireEffect)
            {
                UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireEffect, GetActorLocation());
                if (NiagaraComp)
                {
                    // Sử dụng TWeakObjectPtr để tránh trường hợp component bị xóa trước khi timer chạy hết
                    TWeakObjectPtr<UNiagaraComponent> WeakNiagaraComp = NiagaraComp;
                    FTimerHandle NiagaraTimerHandle;
                    GetWorld()->GetTimerManager().SetTimer(NiagaraTimerHandle, [WeakNiagaraComp]()
                        {
                            if (WeakNiagaraComp.IsValid())
                            {
                                WeakNiagaraComp->DestroyComponent();
                            }
                        }, 5.0f, false);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: FireEffect is nullptr."));
            }

            // Spawn actor vùng "cháy" để gây sát thương khi nhân vật đi vào
            if (FireZoneClass)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = this;
                SpawnParams.Instigator = GetInstigator();
                AFireZone* FireZone = GetWorld()->SpawnActor<AFireZone>(FireZoneClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
                if (FireZone)
                {
                    // Thiết lập thời gian tồn tại của FireZone (ở đây 5 giây)
                    FireZone->SetLifeSpan(5.0f);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("MolotovCocktail: FireZoneClass is nullptr."));
            }

            UE_LOG(LogTemp, Log, TEXT("Molotov Cocktail: Fire activated after delay."));
            // Hủy actor bomb sau khi kích hoạt hiệu ứng
            Destroy();

        }, 3.0f, false);
}

void AItem::HandleUseItem()
{
    // Xử lý sử dụng item tùy theo danh mục loại item
    switch (ItemCategory)
    {
    case EItemCategory::HeathMedicine:
        HandleHealthMedicine();
        break;
    case EItemCategory::StaminaMedicine:
        HandleStaminaMedicine();
        break;
    case EItemCategory::General:
        HandleMolotovCocktail();
        break;
    case EItemCategory::Flash:
        HandleFlashExplosive();
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("This item cannot be used."));
        break;
    }
}

void AItem::HandleFlashExplosive()
{
    GetWorldTimerManager().SetTimer(BombActivationTime, this, &AItem::ExplodeFlash, ActivationTime(3.0f), false);
}

void AItem::HandleMolotovCocktail()
{
    if (IgniteEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), IgniteEffect,GetActorLocation());
    }

    GetWorldTimerManager().SetTimer(BombActivationTime, this, &AItem::MolotovCocktail, ActivationTime(3.0f), false);
}

float AItem::ActivationTime(float Seconds)
{
    return Seconds;
}

void AItem::ThrowBomb(const FVector& TargetLocation, float ProjectileSpeed)
{
    FVector StartLocation = GetActorLocation();
    FVector TossVelocity;

    // Tính toán vector vận tốc cần thiết để đưa bomb từ StartLocation tới TargetLocation
    // bFavorHighArc = false: đường bay thấp; thay đổi thành true nếu muốn đường bay cao hơn.
    bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
        this,
        TossVelocity,
        StartLocation,
        TargetLocation,
        ProjectileSpeed,                     // Vận tốc ban đầu
        false,                               // bFavorHighArc: đường bay thấp
        0.0f,                                // Override gravity: dùng gravity mặc định nếu bằng 0
        0.0f,                                // Collision radius nếu cần (ở đây bỏ qua)
        ESuggestProjVelocityTraceOption::DoNotTrace
    );

    if (bHaveAimSolution)
    {
        // Bật mô phỏng vật lý nếu chưa bật
        if (ItemMesh && !ItemMesh->IsSimulatingPhysics())
        {
            ItemMesh->SetSimulatePhysics(true);
        }
        // Áp dụng impulse dựa vào vận tốc tính được, nhân thêm khối lượng để đảm bảo lực đủ
        if (ItemMesh)
        {
            ItemMesh->AddImpulse(TossVelocity * ItemMesh->GetMass());
        }
        UE_LOG(LogTemp, Log, TEXT("ThrowBomb: Bomb được ném với lực: %s"), *TossVelocity.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ThrowBomb: Không tính được vận tốc ném tới mục tiêu (%s)."), *TargetLocation.ToString());
    }
}

void AItem::ActivateAndThrowBomb(const FVector& TargetLocation, float ProjectileSpeed, bool bIsFlashBomb)
{
    // Ném bomb về phía mục tiêu với đường bay vật lý
    ThrowBomb(TargetLocation, ProjectileSpeed);

    // Thiết lập timer delay kích hoạt hiệu ứng sau thời gian định sẵn
    if (bIsFlashBomb)
    {
        // Kích hoạt flash
        GetWorldTimerManager().SetTimer(BombActivationTime, this, &AItem::ExplodeFlash, ActivationTime(ActivationDelay), false);
    }
    else
    {
        // Với Molotov, hiển thị hiệu ứng Ignite ngay khi kích hoạt
        if (IgniteEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), IgniteEffect, GetActorLocation());
        }
        GetWorldTimerManager().SetTimer(BombActivationTime, this, &AItem::MolotovCocktail, ActivationTime(ActivationDelay), false);
    }
}