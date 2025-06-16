#include "HorrorGame/Actor/NoteActor.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"
#include "HorrorGame/HUD/PlayerHUDWidget.h"
#include "HorrorGame/Widget/MainHUDWidget.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"

ANoteActor::ANoteActor()
{

}

void ANoteActor::BeginPlay()
{
	Super::BeginPlay();
	OriginalTransform = GetActorTransform();
}

void ANoteActor::Interact(AHorrorGameCharacter* Player)
{
    if (!Player) return;

	TogglePlayerHUD(false);

    Player->SetCurrentNoteActor(this);
    HideNote();
    Player->ShowNoteUI(NoteImage, NoteText);
}

void ANoteActor::HideNote()
{
	// ẩn mesh + tắt collision
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ANoteActor::ReturnToOriginal()
{
	TogglePlayerHUD(true);

	// trả về vị trí gốc, hiện lại và bật collision
	SetActorTransform(OriginalTransform);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ANoteActor::TogglePlayerHUD(bool bVisible)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Lấy HUD và gọi đến MainWidget để set visibility
    APlayerHUDWidget* PlayerHUD = Cast<APlayerHUDWidget>(PC->GetHUD());
    if (PlayerHUD && PlayerHUD->MainWidgetInstance)
    {
        PlayerHUD->MainWidgetInstance->SetAllPanelsVisibility(bVisible);
    }

	AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(PC->GetPawn());
	if (Player)
	{
		Player->SetInventoryVisible(bVisible);
	}
}
