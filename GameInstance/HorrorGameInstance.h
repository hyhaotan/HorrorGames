#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HorrorGameInstance.generated.h"

class UPlayerIDManager;
class USteamLobbySubsystem;

UCLASS()
class HORRORGAME_API UHorrorGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    // Global Player ID Manager
    UPROPERTY(BlueprintReadOnly)
    UPlayerIDManager* GlobalPlayerIDManager;

protected:
    void InitializeSteamIntegration();
    void CleanupSteamIntegration();
};