#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class HORRORGAME_API ALobbyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALobbyGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

protected:
    virtual void BeginPlay() override;

private:
    void UpdateMaxPlayers();
    
    UPROPERTY()
    class ULobbyWidget* LobbyWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Lobby")
    TSubclassOf<class ULobbyWidget> LobbyWidgetClass;
};