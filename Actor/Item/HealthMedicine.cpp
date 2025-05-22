// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Actor/Item/HealthMedicine.h"
#include "HorrorGame/HorrorGameCharacter.h"

AHealthMedicine::AHealthMedicine()
{
}

void AHealthMedicine::UseItem()
{
    // Gọi tính lượng hồi máu
    if (FItemData* DataRow = GetItemData())
    {
        float HealAmount = CalculateHealAmount(DataRow->MedicineSize);

        if (AHorrorGameCharacter* Char = Cast<AHorrorGameCharacter>(GetOwner()))
        {
            Char->IncreaseHealth(HealAmount);
            // Hủy item sau khi dùng
            Destroy();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AHealthMedicine::UseItem – Owner không phải là character"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AHealthMedicine::UseItem – Không tìm thấy DataRow"));
    }
}

float AHealthMedicine::CalculateHealAmount(EMedicineSize Size) const
{
    switch (Size)
    {
    case EMedicineSize::Small:
        return 25.f;
    case EMedicineSize::Medium:
        return 50.f;
    case EMedicineSize::Large:
        return 100.f;
    default:
        return 0.f;
    }
}