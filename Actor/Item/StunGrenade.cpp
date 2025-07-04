// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Actor/Item/StunGrenade.h"
#include "HorrorGame/Actor/Component/BomComponent.h"

AStunGrenade::AStunGrenade()
{
}

void AStunGrenade::UseItem()
{
    if (UBomComponent* BomComp = FindComponentByClass<UBomComponent>())
    {
        BomComp->HandleFlashExplosive();
        this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HandleFlashExplosive: BomComponent not found on AItem"));
    }
}
