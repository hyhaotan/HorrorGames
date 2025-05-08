// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Actor/NoteActor.h"
#include "HorrorGame/HorrorGameCharacter.h"

ANoteActor::ANoteActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void ANoteActor::BeginPlay()
{
	Super::BeginPlay();
	OriginalTransform = GetActorTransform();
}

void ANoteActor::Interact(AActor* Interactor)
{
	// Tell the character to show the note UI
	if (AHorrorGameCharacter* C = Cast<AHorrorGameCharacter>(Interactor))
	{
		C->SetCurrentNoteActor(this);
		HideNote();
		C->ShowNoteUI(NoteImage);
	}
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