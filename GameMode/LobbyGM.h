// LobbyGM.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGM.generated.h"

class ALobbyPlayerPlatform;

UCLASS()
class HORRORGAME_API ALobbyGM : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

protected:
    /** Class nhân vật spawn trong lobby */
    UPROPERTY(EditDefaultsOnly, Category = "Lobby")
    TSubclassOf<AActor> LobbyCharacterClass;

private:
    UPROPERTY()
    TArray<ALobbyPlayerPlatform*> Platforms;

    UPROPERTY()
    TMap<APlayerController*, ALobbyPlayerPlatform*> PlayerToPlatform;

    void SetupPlatforms();
    ALobbyPlayerPlatform* FindFreePlatform() const;
};