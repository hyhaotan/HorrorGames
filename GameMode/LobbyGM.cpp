// LobbyGM.cpp
#include "LobbyGM.h"
#include "HorrorGame/Pawn/LobbyPlayerPlatform.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Components/ActorComponent.h"

static bool ParseSlotIndexFromName(const FName& Tag, int32& OutIndex)
{
    const FString S = Tag.ToString();
    if (S.StartsWith(TEXT("Slot")))
    {
        const FString Num = S.Mid(4);
        if (!Num.IsEmpty() && Num.IsNumeric())
        {
            OutIndex = FCString::Atoi(*Num);
            return true;
        }
    }
    return false;
}

void ALobbyGM::BeginPlay()
{
    Super::BeginPlay();
    SetupPlatforms();
}

void ALobbyGM::SetupPlatforms()
{
    Platforms.Empty();

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyPlayerPlatform::StaticClass(), Found);

    struct FEntry { int32 Index; ALobbyPlayerPlatform* P; bool bHasIndex; };
    TArray<FEntry> Indexed;
    TArray<FEntry> Unindexed;

    for (AActor* A : Found)
    {
        if (ALobbyPlayerPlatform* P = Cast<ALobbyPlayerPlatform>(A))
        {
            int32 BestIdx = TNumericLimits<int32>::Max();
            bool bFoundIdx = false;

            // 1) Actor tags
            for (const FName& Tag : P->Tags)
            {
                int32 TagIdx;
                if (ParseSlotIndexFromName(Tag, TagIdx))
                {
                    BestIdx = FMath::Min(BestIdx, TagIdx);
                    bFoundIdx = true;
                }
            }

            // 2) Component tags (phòng trường hợp tag đặt ở Mesh/Component)
            if (!bFoundIdx)
            {
                TArray<UActorComponent*> Comps = P->GetComponents().Array();
                for (UActorComponent* C : Comps)
                {
                    for (const FName& Tag : C->ComponentTags)
                    {
                        int32 TagIdx;
                        if (ParseSlotIndexFromName(Tag, TagIdx))
                        {
                            BestIdx = FMath::Min(BestIdx, TagIdx);
                            bFoundIdx = true;
                        }
                    }
                }
            }

            if (bFoundIdx)
            {
                Indexed.Add({ BestIdx, P, true });
                UE_LOG(LogTemp, Log, TEXT("LobbyGM: Found platform %s -> Slot%d"), *GetNameSafe(P), BestIdx);
            }
            else
            {
                Unindexed.Add({ TNumericLimits<int32>::Max(), P, false });
                UE_LOG(LogTemp, Warning, TEXT("LobbyGM: Platform %s has NO Slot tag. Appending to end."), *GetNameSafe(P));
            }
        }
    }

    Indexed.Sort([](const FEntry& A, const FEntry& B) { return A.Index < B.Index; });

    for (const FEntry& E : Indexed) { Platforms.Add(E.P); }
    for (const FEntry& E : Unindexed) { Platforms.Add(E.P); }

    UE_LOG(LogTemp, Log, TEXT("LobbyGM: Total platforms ready: %d"), Platforms.Num());
}

ALobbyPlayerPlatform* ALobbyGM::FindFreePlatform() const
{
    for (ALobbyPlayerPlatform* P : Platforms)
    {
        if (IsValid(P) && !P->HasPlayer())
            return P;
    }
    return nullptr;
}

void ALobbyGM::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!NewPlayer) return;

    if (Platforms.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Platforms empty in PostLogin. Rescanning..."));
        SetupPlatforms();
    }

    UE_LOG(LogTemp, Log, TEXT("PostLogin: %s, CurrentPlayers=%d"),
        *GetNameSafe(NewPlayer),
        GetGameState<AGameStateBase>() ? GetGameState<AGameStateBase>()->PlayerArray.Num() : -1);

    ALobbyPlayerPlatform* Slot = FindFreePlatform();
    if (!Slot)
    {
        UE_LOG(LogTemp, Warning, TEXT("No free slot for %s"), *GetNameSafe(NewPlayer));
        return;
    }

    if (!LobbyCharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("LobbyCharacterClass is NOT set in GameMode."));
        return;
    }

    Slot->CharacterClass = LobbyCharacterClass;
    Slot->SpawnCharacter(NewPlayer);

    PlayerToPlatform.Add(NewPlayer, Slot);

    UE_LOG(LogTemp, Log, TEXT("Assigned %s -> %s"), *GetNameSafe(NewPlayer), *GetNameSafe(Slot));
}

void ALobbyGM::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (APlayerController* PC = Cast<APlayerController>(Exiting))
    {
        if (ALobbyPlayerPlatform** Found = PlayerToPlatform.Find(PC))
        {
            if (IsValid(*Found))
            {
                (*Found)->Clear();
                UE_LOG(LogTemp, Log, TEXT("Cleared slot of %s"), *GetNameSafe(PC));
            }
            PlayerToPlatform.Remove(PC);
        }
    }
}