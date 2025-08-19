#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "PlayerIDManager.generated.h"

USTRUCT(BlueprintType)
struct FPlayerProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SteamID;

    UPROPERTY(BlueprintReadOnly)
    FString PlayerID;   

    UPROPERTY(BlueprintReadOnly)
    FString DisplayName;      

    UPROPERTY(BlueprintReadOnly)
    FDateTime CreatedDate;

    UPROPERTY(BlueprintReadOnly)
    FDateTime LastNameUpdate;   

    FPlayerProfile()
    {
        SteamID = "";
        PlayerID = "";
        DisplayName = "";
        CreatedDate = FDateTime::Now();
        LastNameUpdate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class HORRORGAME_API UPlayerIDManager : public UObject
{
    GENERATED_BODY()

public:
    UPlayerIDManager();

    // ===== CORE FUNCTIONS =====

    // Đăng ký người chơi mới (tự động tạo Player ID)
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    bool RegisterCurrentPlayer();

    // Lấy Player ID của người chơi hiện tại
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    FString GetMyPlayerID();

    // Lấy profile đầy đủ của người chơi hiện tại
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    FPlayerProfile GetMyProfile();

    // Cập nhật Steam name (sync với Steam profile)
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    bool UpdateSteamName();

    // ===== SEARCH & LOOKUP =====

    // Tìm người chơi bằng Player ID
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    FPlayerProfile FindPlayerByID(const FString& PlayerID);

    // Tìm người chơi bằng Steam display name
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    TArray<FPlayerProfile> FindPlayersByName(const FString& SteamName);

    // ===== NAME MANAGEMENT =====

    // Lấy Steam name hiện tại từ Steam profile
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    FString GetCurrentSteamName();

    // ===== VALIDATION =====

    // Kiểm tra Player ID có hợp lệ không
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    bool IsValidPlayerID(const FString& PlayerID);

    // Kiểm tra Player ID có hợp lệ không
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    bool IsValidDisplayName(const FString& DisplayName);

    // ===== FRIEND SYSTEM HELPERS =====

    // Format cho hiển thị friend (Name + ID)
    UFUNCTION(BlueprintCallable, Category = "Player ID")
    FString GetFriendDisplayFormat(const FString& PlayerID);

protected:
    // Generate random Player ID (Dead by Daylight style)
    FString GenerateRandomPlayerID();

    // Kiểm tra ID có trùng không
    bool IsPlayerIDUnique(const FString& PlayerID);

    // Character sets cho generation
    static const FString LETTERS;
    static const FString NUMBERS;

private:
    // Storage mappings
    UPROPERTY()
    TMap<FString, FPlayerProfile> SteamToProfileMap;

    UPROPERTY()
    TMap<FString, FString> PlayerIDToSteamMap; 

    // Steam interface
    IOnlineIdentityPtr IdentityInterface;

    void InitializeSteamInterface();
    void LoadPlayerDatabase(); 
    void SavePlayerDatabase(); 
};