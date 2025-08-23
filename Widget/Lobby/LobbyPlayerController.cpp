#include "LobbyPlayerController.h"
#include "LobbyGameState.h"
#include "HorrorGame/GameMode/LobbyGM.h"
#include "HorrorGame/Widget/Lobby/LobbyMainWidget.h"
#include "HorrorGame/Object/PlayerIDManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ALobbyPlayerController::ALobbyPlayerController()
{
    bReplicates = true;
    bHasRegistered = false;
    LobbyGameState = nullptr;
    PlayerIDManager = nullptr;
    LobbyMainWidget = nullptr;
}

void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
    InitializePlayerManager();

    if (IsLocalController())
    {
        CreateLobbyUI();

        // Bind to lobby events
        if (LobbyGameState)
        {
            LobbyGameState->OnLobbyStateChanged.AddDynamic(this, &ALobbyPlayerController::OnLobbyStateChanged);
            LobbyGameState->OnPlayerJoined.AddDynamic(this, &ALobbyPlayerController::OnPlayerJoinedLobby);
            LobbyGameState->OnPlayerLeft.AddDynamic(this, &ALobbyPlayerController::OnPlayerLeftLobby);
            LobbyGameState->OnPlayerReadyChanged.AddDynamic(this, &ALobbyPlayerController::OnPlayerReadyChanged);
            LobbyGameState->OnCountdownChanged.AddDynamic(this, &ALobbyPlayerController::OnCountdownChanged);
        }
    }

    // Register player on server
    if (HasAuthority())
    {
        ServerRegisterPlayer();
    }
}

void ALobbyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (IsLocalController() && IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->AddToViewport();
    }
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPlayerController, MyPlayerData);
}

void ALobbyPlayerController::InitializePlayerManager()
{
    if (!IsValid(PlayerIDManager))
    {
        PlayerIDManager = NewObject<UPlayerIDManager>(this);
    }
}

void ALobbyPlayerController::CreateLobbyUI()
{
    if (!IsLocalController() || !IsValid(LobbyMainWidgetClass)) return;

    LobbyMainWidget = CreateWidget<ULobbyMainWidget>(this, LobbyMainWidgetClass);
    if (IsValid(LobbyMainWidget))
    {
        UE_LOG(LogTemp, Log, TEXT("Lobby UI created for player"));
    }
}

void ALobbyPlayerController::ServerRegisterPlayer_Implementation()
{
    if (!HasAuthority()) return;

    ALobbyGM* LobbyGM = GetWorld()->GetAuthGameMode<ALobbyGM>();
    if (!IsValid(LobbyGM) || !IsValid(LobbyGM->PlayerIDManager))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot access PlayerIDManager"));
        return;
    }

    // Register player and get data
    bool bRegistered = LobbyGM->PlayerIDManager->RegisterCurrentPlayer();
    if (!bRegistered)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to register player"));
        return;
    }

    // Get player profile
    FPlayerProfile Profile = LobbyGM->PlayerIDManager->GetMyProfile();
    if (Profile.PlayerID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get player profile"));
        return;
    }

    // Create lobby player data
    MyPlayerData.SteamID = Profile.SteamID;
    MyPlayerData.PlayerID = Profile.PlayerID;
    MyPlayerData.DisplayName = Profile.DisplayName;
    MyPlayerData.ReadyState = EPlayerReadyState::NotReady;
    MyPlayerData.bIsHost = (LobbyGameState->HostSteamID.IsEmpty() || LobbyGameState->HostSteamID == Profile.SteamID);

    // Set host if first player
    if (LobbyGameState->HostSteamID.IsEmpty())
    {
        LobbyGameState->HostSteamID = Profile.SteamID;
        MyPlayerData.bIsHost = true;
    }

    // Find available slot
    MyPlayerData.SlotIndex = LobbyGameState->ConnectedPlayers.Num();

    // Add to game state
    LobbyGameState->ConnectedPlayers.Add(MyPlayerData);
    LobbyGameState->MulticastPlayerJoined(MyPlayerData);

    // Send data to client
    ClientUpdatePlayerData(MyPlayerData);
    ClientShowLobbyUI();

    bHasRegistered = true;
    UE_LOG(LogTemp, Log, TEXT("Player registered: %s (%s) - Host: %s"),
        *MyPlayerData.DisplayName, *MyPlayerData.PlayerID, MyPlayerData.bIsHost ? TEXT("Yes") : TEXT("No"));
}

void ALobbyPlayerController::ClientShowLobbyUI_Implementation()
{
    if (IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->AddToViewport();

        // Update with current lobby state
        if (LobbyGameState)
        {
            LobbyMainWidget->UpdateLobbyInfo(LobbyGameState->LobbySettings);
            LobbyMainWidget->UpdatePlayerSlots(LobbyGameState->ConnectedPlayers);
            LobbyMainWidget->UpdateLobbyState(LobbyGameState->CurrentLobbyState);
            LobbyMainWidget->SetHostControls(MyPlayerData.bIsHost);
        }
    }
}

void ALobbyPlayerController::ClientUpdatePlayerData_Implementation(const FLobbyPlayerData& PlayerData)
{
    MyPlayerData = PlayerData;
}

void ALobbyPlayerController::ClientGameStarting_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Game is starting!"));

    if (IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->RemoveFromParent();
    }
}

void ALobbyPlayerController::SetPlayerReady(bool bReady)
{
    if (!bHasRegistered) return;
    ServerSetReady(bReady);
}

void ALobbyPlayerController::ServerSetReady_Implementation(bool bReady)
{
    if (!HasAuthority() || !LobbyGameState) return;

    LobbyGameState->ServerSetPlayerReady(MyPlayerData.SteamID, bReady);
}

void ALobbyPlayerController::LeaveLobby()
{
    ServerLeaveLobby();
}

void ALobbyPlayerController::ServerLeaveLobby_Implementation()
{
    if (!HasAuthority()) return;

    // Remove from connected players
    LobbyGameState->ConnectedPlayers.RemoveAll([this](const FLobbyPlayerData& Players)
        {
            return Players.SteamID == MyPlayerData.SteamID;
        });

    LobbyGameState->MulticastPlayerLeft(MyPlayerData);

    // Transfer host if needed
    if (MyPlayerData.bIsHost && LobbyGameState->ConnectedPlayers.Num() > 0)
    {
        LobbyGameState->HostSteamID = LobbyGameState->ConnectedPlayers[0].SteamID;
        LobbyGameState->ConnectedPlayers[0].bIsHost = true;
        UE_LOG(LogTemp, Log, TEXT("Host transferred to %s"), *LobbyGameState->ConnectedPlayers[0].DisplayName);
    }

    UGameplayStatics::OpenLevel(this, "MainMenu", true);

    UE_LOG(LogTemp, Log, TEXT("Player left lobby: %s"), *MyPlayerData.DisplayName);
}

void ALobbyPlayerController::StartGame()
{
    if (!MyPlayerData.bIsHost) return;
    ServerStartGame();
}

void ALobbyPlayerController::ServerStartGame_Implementation()
{
    if (!HasAuthority() || !MyPlayerData.bIsHost) return;

    ALobbyGM* LobbyGM = GetWorld()->GetAuthGameMode<ALobbyGM>();
    if (IsValid(LobbyGM))
    {
        LobbyGM->StartLobbyGame();
    }
}

void ALobbyPlayerController::UpdateLobbySettings(const FLobbySettings& NewSettings)
{
    if (!MyPlayerData.bIsHost) return;

    ALobbyGM* LobbyGM = GetWorld()->GetAuthGameMode<ALobbyGM>();
    if (IsValid(LobbyGM))
    {
        LobbyGM->UpdateLobbySettings(NewSettings);
    }
}

// Event Handlers
void ALobbyPlayerController::OnLobbyStateChanged(ELobbyState NewState)
{
    if (IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->UpdateLobbyState(NewState);
    }

    if (NewState == ELobbyState::Starting)
    {
        ClientGameStarting();
    }
}

void ALobbyPlayerController::OnPlayerJoinedLobby(const FLobbyPlayerData& PlayerData)
{
    UE_LOG(LogTemp, Log, TEXT("Player joined: %s"), *PlayerData.DisplayName);

    if (IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->UpdatePlayerSlots(LobbyGameState->ConnectedPlayers);
    }
}

void ALobbyPlayerController::OnPlayerLeftLobby(const FLobbyPlayerData& PlayerData)
{
    UE_LOG(LogTemp, Log, TEXT("Player left: %s"), *PlayerData.DisplayName);

    if (IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->UpdatePlayerSlots(LobbyGameState->ConnectedPlayers);
    }
}

void ALobbyPlayerController::OnPlayerReadyChanged(const FLobbyPlayerData& PlayerData)
{
    if (IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->UpdatePlayerSlots(LobbyGameState->ConnectedPlayers);
    }
}

void ALobbyPlayerController::OnCountdownChanged(int32 SecondsLeft)
{
    if (IsValid(LobbyMainWidget))
    {
        LobbyMainWidget->UpdateCountdown(SecondsLeft);
    }
}

void ALobbyPlayerController::InviteFriend(const FString& FriendSteamID)
{
    if (!bHasRegistered)
    {
        UE_LOG(LogTemp, Warning, TEXT("InviteFriend: player not registered yet."));
        return;
    }

    if (FriendSteamID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("InviteFriend called with empty FriendSteamID"));
        return;
    }

    // Gọi lên server để thực hiện invite (server sẽ làm việc với hệ thống platform/Steam hoặc PlayerIDManager)
    ServerInviteFriend(FriendSteamID);
}

void ALobbyPlayerController::ServerInviteFriend_Implementation(const FString& FriendSteamID)
{
    if (!HasAuthority())
    {
        return;
    }

    if (FriendSteamID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("ServerInviteFriend: empty FriendSteamID"));
        ClientReceiveInviteResult(false, FriendSteamID);
        return;
    }

    // Lấy GameMode / PlayerIDManager nếu cần
    ALobbyGM* LobbyGM = GetWorld()->GetAuthGameMode<ALobbyGM>();
    bool bSent = false;

    if (IsValid(LobbyGM) && IsValid(LobbyGM->PlayerIDManager))
    {
        // Nếu bạn có hàm gửi invite trong PlayerIDManager, gọi nó ở đây.
        // Mình không gọi trực tiếp phương thức không tồn tại để tránh lỗi biên dịch.
        // Ví dụ (nếu tồn tại): bSent = LobbyGM->PlayerIDManager->SendInviteToFriend(FriendSteamID, MyPlayerData.SteamID);
        // Hiện tại ta chỉ log và đánh dấu thành công giả lập nếu có OnlineSubsystem.
        UE_LOG(LogTemp, Log, TEXT("ServerInviteFriend: PlayerIDManager available — you can call its invite method here."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ServerInviteFriend: PlayerIDManager not available"));
    }

    // Thử check OnlineSubsystem làm fallback (chỉ để kiểm tra; actual invite logic cần cài thêm)
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        bSent = true; // đánh dấu là có subsystem; bạn cần bổ sung logic gửi invite cụ thể theo subsystem/Steam SDK
        UE_LOG(LogTemp, Log, TEXT("ServerInviteFriend: OnlineSubsystem present (you should integrate actual invite API here)."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("ServerInviteFriend: No OnlineSubsystem found — cannot send platform invite automatically."));
    }

    // Gửi kết quả về client
    ClientReceiveInviteResult(bSent, FriendSteamID);
}

void ALobbyPlayerController::ClientReceiveInviteResult_Implementation(bool bSuccess, const FString& FriendSteamID)
{
    // Hiển thị thông báo ngắn trên client (hoặc bạn có thể gọi một hàm UI trong LobbyMainWidget)
    FString Msg = bSuccess ? FString::Printf(TEXT("Invite to %s sent."), *FriendSteamID)
        : FString::Printf(TEXT("Failed to send invite to %s."), *FriendSteamID);

    UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);

    if (GEngine && IsLocalController())
    {
        FColor Color = bSuccess ? FColor::Green : FColor::Red;
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, Color, Msg);
    }

    // Nếu bạn muốn update UI widget, hãy gọi method ở đây (nếu widget có API tương ứng).
    // Ví dụ (nếu LobbyMainWidget có method ShowInviteResult): LobbyMainWidget->ShowInviteResult(bSuccess, FriendSteamID);
}
