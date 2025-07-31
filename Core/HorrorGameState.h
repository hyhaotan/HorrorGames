#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HorrorGameState.generated.h"

/**
 * Game state class for the horror game.
 * Handles multiplayer state information like max players and current players.
 */
UCLASS()
class HORRORGAME_API AHorrorGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AHorrorGameState();

    /** Maximum number of players allowed in the game */
    UPROPERTY(Replicated)
    int32 MaxPlayers;

    /** Current number of players in the game */
    UPROPERTY(Replicated)
    int32 CurrentPlayers;

    /** Sets the maximum number of players allowed */
    void SetMaxPlayers(int32 NewMaxPlayers);

    /** Gets the maximum number of players allowed */
    int32 GetMaxPlayers() const { return MaxPlayers; }

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};