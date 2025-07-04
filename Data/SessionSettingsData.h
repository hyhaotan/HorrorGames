#pragma once

#include "CoreMinimal.h"
#include "SessionSettingsData.generated.h"

USTRUCT(BlueprintType)
struct FSessionSettingsData
{
    GENERATED_BODY()

    /** Tên phòng */
    UPROPERTY(BlueprintReadWrite)
    FString RoomName;

    /** Mật khẩu (để join) */
    UPROPERTY(BlueprintReadWrite)
    FString Password;

    /** Số người chơi tối đa (2, 4, 8, …) */
    UPROPERTY(BlueprintReadWrite)
    int32 MaxPlayers = 4;

    /** Tên map gameplay mà host muốn chơi ngay sau lobby */
    UPROPERTY(BlueprintReadWrite)
    FString MapName;

    FSessionSettingsData()
        : RoomName(TEXT("MyRoom"))
        , Password(TEXT(""))
        , MaxPlayers(4)
        , MapName(TEXT("DefaultMap"))
    {
    }
};
