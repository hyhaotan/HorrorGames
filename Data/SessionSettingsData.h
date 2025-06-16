#pragma once

#include "CoreMinimal.h"
#include "SessionSettingsData.generated.h"

/**
 * Struct này chứa toàn bộ thông tin do người chơi nhập khi tạo lobby:
 * - RoomName: Tên phòng
 * - Password: Mật khẩu (có thể rỗng nếu không cần)
 * - MaxPlayers: Số lượng người chơi tối đa (ví dụ 2, 4)
 * - MapName: Tên map gameplay (khi host start game, sẽ load map này)
 */
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
