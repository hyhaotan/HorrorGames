#include "HorrorGameInstance.h"
#include "HorrorGame/Object/PlayerIDManager.h"
#include "SteamLobbySubsystem.h"
#include "Engine/Engine.h"

void UHorrorGameInstance::Init()
{
    Super::Init();

    // Create global PlayerIDManager
    GlobalPlayerIDManager = NewObject<UPlayerIDManager>(this);

    InitializeSteamIntegration();

    UE_LOG(LogTemp, Log, TEXT("HorrorGameInstance initialized"));
}

void UHorrorGameInstance::Shutdown()
{
    CleanupSteamIntegration();
    Super::Shutdown();
}

void UHorrorGameInstance::InitializeSteamIntegration()
{
    // Steam integration is handled by subsystems
    UE_LOG(LogTemp, Log, TEXT("Steam integration initialized"));
}

void UHorrorGameInstance::CleanupSteamIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Steam integration cleaned up"));
}