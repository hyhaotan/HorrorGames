#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LobbyType.generated.h"

UENUM(BlueprintType)
enum class ELobbyState : uint8
{
    Waiting,
    CountingDown,
    Starting,
    InGame
};

UENUM(BlueprintType)
enum class EPlayerReadyState : uint8
{
    NotReady,
    Ready,
    Loading
};

USTRUCT(BlueprintType)
struct FLobbyPlayerData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString SteamID;

    UPROPERTY(BlueprintReadWrite)
    FString PlayerID;

    UPROPERTY(BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite)
    EPlayerReadyState ReadyState;

    UPROPERTY(BlueprintReadWrite)
    int32 SlotIndex;

    UPROPERTY(BlueprintReadWrite)
    bool bIsHost;

    UPROPERTY(BlueprintReadWrite)
    float Ping;

    FLobbyPlayerData()
    {
        ReadyState = EPlayerReadyState::NotReady;
        SlotIndex = -1;
        bIsHost = false;
        Ping = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FLobbySettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 MaxPlayers;

    UPROPERTY(BlueprintReadWrite)
    FString LobbyName;

    UPROPERTY(BlueprintReadWrite)
    bool bIsPrivate;

    UPROPERTY(BlueprintReadWrite)
    FString Password;

    UPROPERTY(BlueprintReadWrite)
    FString GameMode;

    UPROPERTY(BlueprintReadWrite)
    FString MapName;

    FLobbySettings()
    {
        MaxPlayers = 4;
        LobbyName = TEXT("New Lobby");
        bIsPrivate = false;
        GameMode = TEXT("Classic");
        MapName = TEXT("DefaultMap");
    }
};
