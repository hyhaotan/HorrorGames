#include "PlayerIDManager.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/Engine.h"

// Dead by Daylight style character sets
const FString UPlayerIDManager::LETTERS = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
const FString UPlayerIDManager::NUMBERS = TEXT("0123456789");

UPlayerIDManager::UPlayerIDManager()
{
    InitializeSteamInterface();
    LoadPlayerDatabase();
}

void UPlayerIDManager::InitializeSteamInterface()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        IdentityInterface = OnlineSubsystem->GetIdentityInterface();
        UE_LOG(LogTemp, Log, TEXT("PlayerIDManager: Connected to %s"),
            *OnlineSubsystem->GetSubsystemName().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerIDManager: No Online Subsystem found!"));
    }
}

bool UPlayerIDManager::RegisterCurrentPlayer()
{
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("No Steam connection available"));
        return false;
    }

    // Lấy Steam ID của người chơi hiện tại
    FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot get current player's Steam ID"));
        return false;
    }

    FString CurrentSteamID = UserId->ToString();

    // Kiểm tra đã đăng ký chưa
    if (SteamToProfileMap.Contains(CurrentSteamID))
    {
        UE_LOG(LogTemp, Log, TEXT("Player already registered: %s"), *CurrentSteamID);
        return true; // Already registered
    }

    // Tạo Player ID mới (Dead by Daylight style)
    FString NewPlayerID = GenerateRandomPlayerID();
    if (NewPlayerID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to generate unique Player ID"));
        return false;
    }

    // Lấy Steam display name
    FString SteamName = IdentityInterface->GetPlayerNickname(0);
    if (SteamName.IsEmpty())
    {
        SteamName = TEXT("Unknown Player");
    }

    // Tạo profile mới
    FPlayerProfile NewProfile;
    NewProfile.SteamID = CurrentSteamID;
    NewProfile.PlayerID = NewPlayerID;
    NewProfile.DisplayName = SteamName;  // Always use Steam name
    NewProfile.CreatedDate = FDateTime::Now();
    NewProfile.LastNameUpdate = FDateTime::Now();

    // Lưu vào mappings
    SteamToProfileMap.Add(CurrentSteamID, NewProfile);
    PlayerIDToSteamMap.Add(NewPlayerID, CurrentSteamID);

    UE_LOG(LogTemp, Log, TEXT("Player registered - ID: %s, Steam: %s, Name: %s"),
        *NewPlayerID, *CurrentSteamID, *SteamName);

    SavePlayerDatabase();
    return true;
}

FString UPlayerIDManager::GenerateRandomPlayerID()
{
    const int32 MAX_ATTEMPTS = 10000;
    int32 Attempts = 0;

    while (Attempts < MAX_ATTEMPTS)
    {
        FString NewID;
        NewID.Reserve(6);

        // Dead by Daylight format: 3 letters + 3 numbers (e.g., ABC123)

        // Generate 3 random letters
        for (int32 i = 0; i < 3; i++)
        {
            int32 RandomIndex = FMath::RandRange(0, LETTERS.Len() - 1);
            NewID += LETTERS[RandomIndex];
        }

        // Generate 3 random numbers  
        for (int32 i = 0; i < 3; i++)
        {
            int32 RandomIndex = FMath::RandRange(0, NUMBERS.Len() - 1);
            NewID += NUMBERS[RandomIndex];
        }

        // Check if unique
        if (IsPlayerIDUnique(NewID))
        {
            UE_LOG(LogTemp, Log, TEXT("Generated unique Player ID: %s"), *NewID);
            return NewID;
        }

        Attempts++;
    }

    UE_LOG(LogTemp, Error, TEXT("Failed to generate unique Player ID after %d attempts"), MAX_ATTEMPTS);
    return TEXT("");
}

bool UPlayerIDManager::IsPlayerIDUnique(const FString& PlayerID)
{
    return !PlayerIDToSteamMap.Contains(PlayerID);
}

FString UPlayerIDManager::GetMyPlayerID()
{
    if (!IdentityInterface.IsValid())
    {
        return TEXT("");
    }

    FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        return TEXT("");
    }

    FString CurrentSteamID = UserId->ToString();

    if (SteamToProfileMap.Contains(CurrentSteamID))
    {
        return SteamToProfileMap[CurrentSteamID].PlayerID;
    }

    return TEXT("");
}

FPlayerProfile UPlayerIDManager::GetMyProfile()
{
    if (!IdentityInterface.IsValid())
    {
        return FPlayerProfile();
    }

    FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        return FPlayerProfile();
    }

    FString CurrentSteamID = UserId->ToString();

    if (SteamToProfileMap.Contains(CurrentSteamID))
    {
        return SteamToProfileMap[CurrentSteamID];
    }

    return FPlayerProfile();
}

FPlayerProfile UPlayerIDManager::FindPlayerByID(const FString& PlayerID)
{
    if (!IsValidPlayerID(PlayerID))
    {
        return FPlayerProfile();
    }

    if (PlayerIDToSteamMap.Contains(PlayerID))
    {
        FString SteamID = PlayerIDToSteamMap[PlayerID];
        if (SteamToProfileMap.Contains(SteamID))
        {
            return SteamToProfileMap[SteamID];
        }
    }

    return FPlayerProfile();
}

TArray<FPlayerProfile> UPlayerIDManager::FindPlayersByName(const FString& SteamName)
{
    TArray<FPlayerProfile> Results;

    for (const auto& Pair : SteamToProfileMap)
    {
        const FPlayerProfile& Profile = Pair.Value;

        // Search only in Steam display name
        if (Profile.DisplayName.Contains(SteamName))
        {
            Results.Add(Profile);
        }
    }

    return Results;
}

bool UPlayerIDManager::UpdateSteamName()
{
    if (!IdentityInterface.IsValid())
    {
        return false;
    }

    FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        return false;
    }

    FString CurrentSteamID = UserId->ToString();

    if (SteamToProfileMap.Contains(CurrentSteamID))
    {
        // Get current Steam name
        FString CurrentSteamName = IdentityInterface->GetPlayerNickname(0);
        if (CurrentSteamName.IsEmpty())
        {
            CurrentSteamName = TEXT("Unknown Player");
        }

        FPlayerProfile& Profile = SteamToProfileMap[CurrentSteamID];

        // Update only if name has changed
        if (Profile.DisplayName != CurrentSteamName)
        {
            FString OldName = Profile.DisplayName;
            Profile.DisplayName = CurrentSteamName;
            Profile.LastNameUpdate = FDateTime::Now();

            UE_LOG(LogTemp, Log, TEXT("Steam name updated: %s -> %s (Player ID: %s)"),
                *OldName, *CurrentSteamName, *Profile.PlayerID);

            SavePlayerDatabase();
            return true;
        }
    }

    return false;
}

FString UPlayerIDManager::GetCurrentSteamName()
{
    if (!IdentityInterface.IsValid())
    {
        return TEXT("Unknown Player");
    }

    FString SteamName = IdentityInterface->GetPlayerNickname(0);
    return SteamName.IsEmpty() ? TEXT("Unknown Player") : SteamName;
}

bool UPlayerIDManager::IsValidPlayerID(const FString& PlayerID)
{
    // Must be exactly 6 characters
    if (PlayerID.Len() != 6)
    {
        return false;
    }

    // First 3 must be letters (A-Z)
    for (int32 i = 0; i < 3; i++)
    {
        TCHAR Char = PlayerID[i];
        if (!FChar::IsAlpha(Char) || !FChar::IsUpper(Char))
        {
            return false;
        }
    }

    // Last 3 must be numbers (0-9)
    for (int32 i = 3; i < 6; i++)
    {
        TCHAR Char = PlayerID[i];
        if (!FChar::IsDigit(Char))
        {
            return false;
        }
    }

    return true;
}

bool UPlayerIDManager::IsValidDisplayName(const FString& DisplayName)
{
    // Since we only use Steam names, this validates Steam name format
    if (DisplayName.Len() < 1 || DisplayName.Len() > 32) // Steam's limit
    {
        return false;
    }

    // Steam allows most characters, so minimal validation
    FString Trimmed = DisplayName.TrimStartAndEnd();
    return !Trimmed.IsEmpty();
}

FString UPlayerIDManager::GetFriendDisplayFormat(const FString& PlayerID)
{
    FPlayerProfile Profile = FindPlayerByID(PlayerID);
    if (Profile.PlayerID.IsEmpty())
    {
        return TEXT("Unknown Player");
    }

    // Format: "SteamName (ABC123)" for friend lists
    return FString::Printf(TEXT("%s (%s)"), *Profile.DisplayName, *Profile.PlayerID);
}

void UPlayerIDManager::LoadPlayerDatabase()
{
    // In production: Load from server database
    // For now, just initialize empty
    UE_LOG(LogTemp, Log, TEXT("Player database loaded (empty for demo)"));
}

void UPlayerIDManager::SavePlayerDatabase()
{
    // In production: Save to server database
    UE_LOG(LogTemp, Log, TEXT("Player database saved (%d players)"), SteamToProfileMap.Num());
}