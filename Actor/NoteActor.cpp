#include "HorrorGame/Actor/NoteActor.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/WidgetAnimation.h"

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
	// trả về vị trí gốc, hiện lại và bật collision
	SetActorTransform(OriginalTransform);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}