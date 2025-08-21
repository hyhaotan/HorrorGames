#include "LobbyGM.h"
#include "HorrorGame/Pawn/LobbyPlayerPlatform.h"
#include "HorrorGame/Widget/Lobby/LobbyGameState.h"
#include "HorrorGame/Widget/Lobby/LobbyPlayerController.h"
#include "HorrorGame/Object/PlayerIDManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ALobbyGM::ALobbyGM()
{
    GameStateClass = ALobbyGameState::StaticClass();
    PlayerControllerClass = ALobbyPlayerController::StaticClass();

    PlayerIDManager = CreateDefaultSubobject<UPlayerIDManager>(TEXT("PlayerIDManager"));
    bIsGameStarting = false;

    DefaultLobbySettings.MaxPlayers = 4;
    DefaultLobbySettings.LobbyName = TEXT("Horror Game Lobby");
    DefaultLobbySettings.GameMode = TEXT("Classic");
    DefaultLobbySettings.MapName = TEXT("GameLevel");
    GameLevelToLoad = TEXT("/Game/Maps/GameLevel");
}

void ALobbyGM::BeginPlay()
{
    Super::BeginPlay();

    LobbyGameState = GetGameState<ALobbyGameState>();
    if (LobbyGameState)
    {
        LobbyGameState->LobbySettings = DefaultLobbySettings;
    }

    SetupPlatforms();
}

void ALobbyGM::PostLogin(APlayerController* NewPlayer)
{
    if (!CanPlayerJoin(NewPlayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player cannot join - lobby full or game starting"));
        return;
    }

    Super::PostLogin(NewPlayer);

    ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(NewPlayer);
    if (!IsValid(LobbyPC))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid player controller type"));
        return;
    }

    // Assign to platform
    ALobbyPlayerPlatform* Platform = FindFreePlatform();
    if (IsValid(Platform))
    {
        AssignPlayerToPlatform(LobbyPC, Platform);
    }

    // Store reference
    UE_LOG(LogTemp, Log, TEXT("Player logged in: %s"), *GetNameSafe(NewPlayer));
}

void ALobbyGM::Logout(AController* Exiting)
{
    ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(Exiting);
    if (IsValid(PC))
    {
        // Remove from platform
        RemovePlayerFromPlatform(PC);

        // Remove from steam mapping
        if (!PC->MyPlayerData.SteamID.IsEmpty())
        {
            SteamIDToPlayerController.Remove(PC->MyPlayerData.SteamID);
        }
    }

    Super::Logout(Exiting);
}

void ALobbyGM::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    // Don't call parent - we handle spawning in PostLogin
    UE_LOG(LogTemp, Log, TEXT("HandleStartingNewPlayer: %s"), *GetNameSafe(NewPlayer));
}

bool ALobbyGM::CanPlayerJoin(APlayerController* NewPlayer)
{
    if (!LobbyGameState) return false;

    // Check if lobby is full
    if (LobbyGameState->ConnectedPlayers.Num() >= LobbyGameState->LobbySettings.MaxPlayers)
    {
        return false;
    }

    // Check if game is starting
    if (bIsGameStarting || LobbyGameState->CurrentLobbyState == ELobbyState::Starting)
    {
        return false;
    }

    return true;
}

void ALobbyGM::SetupPlatforms()
{
    Platforms.Empty();

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALobbyPlayerPlatform::StaticClass(), Found);

    struct FEntry { int32 Index; ALobbyPlayerPlatform* P; bool bHasIndex; };
    TArray<FEntry> Indexed;
    TArray<FEntry> Unindexed;

    auto ParseSlotIndexFromName = [](const FName& Tag, int32& OutIndex) -> bool
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
        };

    for (AActor* A : Found)
    {
        if (ALobbyPlayerPlatform* P = Cast<ALobbyPlayerPlatform>(A))
        {
            int32 BestIdx = TNumericLimits<int32>::Max();
            bool bFoundIdx = false;

            // Check actor tags
            for (const FName& Tag : P->Tags)
            {
                int32 TagIdx;
                if (ParseSlotIndexFromName(Tag, TagIdx))
                {
                    BestIdx = FMath::Min(BestIdx, TagIdx);
                    bFoundIdx = true;
                }
            }

            // Check component tags
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
            }
            else
            {
                Unindexed.Add({ TNumericLimits<int32>::Max(), P, false });
            }
        }
    }

    Indexed.Sort([](const FEntry& A, const FEntry& B) { return A.Index < B.Index; });

    for (const FEntry& E : Indexed) { Platforms.Add(E.P); }
    for (const FEntry& E : Unindexed) { Platforms.Add(E.P); }

    UE_LOG(LogTemp, Log, TEXT("LobbyGM: Setup %d platforms"), Platforms.Num());
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

void ALobbyGM::AssignPlayerToPlatform(ALobbyPlayerController* Player, ALobbyPlayerPlatform* Platform)
{
    if (!IsValid(Player) || !IsValid(Platform)) return;

    Platform->CharacterClass = LobbyCharacterClass;
    Platform->SpawnCharacter(Player);
    PlayerToPlatform.Add(Player, Platform);

    UE_LOG(LogTemp, Log, TEXT("Assigned %s to platform"), *GetNameSafe(Player));
}

void ALobbyGM::RemovePlayerFromPlatform(APlayerController* Player)
{
    if (ALobbyPlayerPlatform** Found = PlayerToPlatform.Find(Player))
    {
        if (IsValid(*Found))
        {
            (*Found)->Clear();
        }
        PlayerToPlatform.Remove(Player);
    }
}

bool ALobbyGM::StartLobbyGame()
{
    if (!HasAuthority() || bIsGameStarting) return false;

    if (!LobbyGameState || !LobbyGameState->CanStartGame())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start game - not all players ready"));
        return false;
    }

    bIsGameStarting = true;
    LobbyGameState->UpdateLobbyState(ELobbyState::Starting);

    // Notify all players
    for (const FLobbyPlayerData& Player : LobbyGameState->ConnectedPlayers)
    {
        if (ALobbyPlayerController** PC = SteamIDToPlayerController.Find(Player.SteamID))
        {
            if (IsValid(*PC))
            {
                (*PC)->ClientGameStarting();
            }
        }
    }

    // Travel to game level
    FString TravelURL = GameLevelToLoad + TEXT("?listen");
    GetWorld()->ServerTravel(TravelURL);

    return true;
}

void ALobbyGM::UpdateLobbySettings(const FLobbySettings& NewSettings)
{
    if (!HasAuthority() || !LobbyGameState) return;

    LobbyGameState->ServerUpdateLobbySettings(NewSettings);
}

void ALobbyGM::KickPlayer(const FString& SteamID)
{
    if (!HasAuthority()) return;

    if (ALobbyPlayerController** PC = SteamIDToPlayerController.Find(SteamID))
    {
        if (IsValid(*PC))
        {
            (*PC)->ClientTravel(TEXT("/Game/Maps/MainMenu"), ETravelType::TRAVEL_Absolute);
        }
    }
}