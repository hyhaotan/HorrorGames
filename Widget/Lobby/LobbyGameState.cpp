#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ALobbyGameState::ALobbyGameState()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    CurrentLobbyState = ELobbyState::Waiting;
    CountdownTimer = 0;

    // Default settings
    LobbySettings.MaxPlayers = 4;
    LobbySettings.LobbyName = TEXT("New Lobby");
    LobbySettings.bIsPrivate = false;
    LobbySettings.GameMode = TEXT("Classic");
    LobbySettings.MapName = TEXT("DefaultMap");
}

void ALobbyGameState::BeginPlay()
{
    Super::BeginPlay();
    UpdateLobbyState(ELobbyState::Waiting);
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALobbyGameState, CurrentLobbyState);
    DOREPLIFETIME(ALobbyGameState, LobbySettings);
    DOREPLIFETIME(ALobbyGameState, ConnectedPlayers);
    DOREPLIFETIME(ALobbyGameState, CountdownTimer);
    DOREPLIFETIME(ALobbyGameState, HostSteamID);
}

FLobbyPlayerData ALobbyGameState::FindPlayerBySteamID(const FString& SteamID)
{
    for (const FLobbyPlayerData& Player : ConnectedPlayers)
    {
        if (Player.SteamID == SteamID)
        {
            return Player;
        }
    }
    return FLobbyPlayerData();
}

bool ALobbyGameState::IsPlayerHost(const FString& SteamID)
{
    return HostSteamID == SteamID;
}

int32 ALobbyGameState::GetReadyPlayerCount()
{
    int32 ReadyCount = 0;
    for (const FLobbyPlayerData& Player : ConnectedPlayers)
    {
        if (Player.ReadyState == EPlayerReadyState::Ready)
        {
            ReadyCount++;
        }
    }
    return ReadyCount;
}

bool ALobbyGameState::CanStartGame()
{
    if (ConnectedPlayers.Num() < 2) // Minimum players
        return false;

    int32 ReadyCount = GetReadyPlayerCount();
    return ReadyCount == ConnectedPlayers.Num();
}

TArray<FLobbyPlayerData> ALobbyGameState::GetPlayersInSlotOrder()
{
    TArray<FLobbyPlayerData> SortedPlayers = ConnectedPlayers;
    SortedPlayers.Sort([](const FLobbyPlayerData& A, const FLobbyPlayerData& B)
        {
            return A.SlotIndex < B.SlotIndex;
        });
    return SortedPlayers;
}

void ALobbyGameState::ServerSetPlayerReady_Implementation(const FString& SteamID, bool bReady)
{
    if (!HasAuthority()) return;

    for (FLobbyPlayerData& Player : ConnectedPlayers)
    {
        if (Player.SteamID == SteamID)
        {
            Player.ReadyState = bReady ? EPlayerReadyState::Ready : EPlayerReadyState::NotReady;
            MulticastPlayerReadyChanged(Player);

            UE_LOG(LogTemp, Log, TEXT("Player %s ready state: %s"),
                *Player.DisplayName, bReady ? TEXT("Ready") : TEXT("Not Ready"));
            break;
        }
    }

    // Auto start countdown if all ready
    if (CanStartGame() && CurrentLobbyState == ELobbyState::Waiting)
    {
        ServerStartCountdown();
    }
    // Cancel countdown if someone unready
    else if (!CanStartGame() && CurrentLobbyState == ELobbyState::CountingDown)
    {
        ServerCancelCountdown();
    }
}

void ALobbyGameState::ServerStartCountdown_Implementation()
{
    if (!HasAuthority() || !CanStartGame()) return;

    UpdateLobbyState(ELobbyState::CountingDown);
    CountdownTimer = COUNTDOWN_DURATION;
    StartCountdownTimer();
}

void ALobbyGameState::ServerCancelCountdown_Implementation()
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
    UpdateLobbyState(ELobbyState::Waiting);
    CountdownTimer = 0;
    OnCountdownChanged.Broadcast(0);
}

void ALobbyGameState::ServerUpdateLobbySettings_Implementation(const FLobbySettings& NewSettings)
{
    if (!HasAuthority()) return;

    LobbySettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Lobby settings updated: %s"), *LobbySettings.LobbyName);
}

void ALobbyGameState::MulticastUpdateLobbyState_Implementation(ELobbyState NewState)
{
    CurrentLobbyState = NewState;
    OnLobbyStateChanged.Broadcast(NewState);
}

void ALobbyGameState::MulticastPlayerJoined_Implementation(const FLobbyPlayerData& PlayerData)
{
    OnPlayerJoined.Broadcast(PlayerData);
}

void ALobbyGameState::MulticastPlayerLeft_Implementation(const FLobbyPlayerData& PlayerData)
{
    OnPlayerLeft.Broadcast(PlayerData);
}

void ALobbyGameState::MulticastPlayerReadyChanged_Implementation(const FLobbyPlayerData& PlayerData)
{
    OnPlayerReadyChanged.Broadcast(PlayerData);
}

void ALobbyGameState::UpdateLobbyState(ELobbyState NewState)
{
    if (CurrentLobbyState != NewState)
    {
        CurrentLobbyState = NewState;
        MulticastUpdateLobbyState(NewState);
    }
}

void ALobbyGameState::StartCountdownTimer()
{
    GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ALobbyGameState::UpdateCountdown, 1.0f, true);
}

void ALobbyGameState::UpdateCountdown()
{
    CountdownTimer--;
    OnCountdownChanged.Broadcast(CountdownTimer);

    UE_LOG(LogTemp, Log, TEXT("Countdown: %d"), CountdownTimer);

    if (CountdownTimer <= 0)
    {
        HandleCountdownFinished();
    }
}

void ALobbyGameState::HandleCountdownFinished()
{
    GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
    UpdateLobbyState(ELobbyState::Starting);

    UE_LOG(LogTemp, Log, TEXT("Countdown finished - Starting game!"));
}