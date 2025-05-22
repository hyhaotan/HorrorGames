// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Actor/Item/MolotovCocktail.h"
#include "HorrorGame/Actor/Component/BomComponent.h"

AMolotovCocktail::AMolotovCocktail()
{
}

void AMolotovCocktail::UseItem()
{
    if (UBomComponent* BomComp = FindComponentByClass<UBomComponent>())
    {
        BomComp->HandleMolotovCocktail();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HandleMolotovCocktail: BomComponent not found on AItem"));
    }
}
