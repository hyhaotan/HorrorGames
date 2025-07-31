#include "HorrorGameState.h"
#include "Net/UnrealNetwork.h"

AHorrorGameState::AHorrorGameState()
{
    MaxPlayers = 4;
    CurrentPlayers = 0;
}

void AHorrorGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AHorrorGameState, MaxPlayers);
    DOREPLIFETIME(AHorrorGameState, CurrentPlayers);
}

void AHorrorGameState::SetMaxPlayers(int32 NewMaxPlayers)
{
    if (HasAuthority())
    {
        MaxPlayers = NewMaxPlayers;
    }
}