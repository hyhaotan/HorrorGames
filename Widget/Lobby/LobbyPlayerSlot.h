#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "LobbyPlayerSlot.generated.h"

class UInviteFriendWidget;

UCLASS()
class HORRORGAME_API ULobbyPlayerSlot : public UUserWidget
{
    GENERATED_BODY()

public:
    // Ghi đè
    virtual void NativeConstruct() override;

    // Thiết lập thông tin người chơi cho slot
    UFUNCTION(BlueprintCallable)
    void SetPlayerInfo(const FString& PlayerName, bool bInIsReady);

    // Đặt slot về trạng thái rỗng
    UFUNCTION(BlueprintCallable)
    void SetEmpty();

    // Trả về trạng thái có người hay không
    UFUNCTION(BlueprintCallable)
    bool IsOccupied() const { return bIsOccupied; }

    // Lấy tên người chơi hiện tại (có thể rỗng)
    UFUNCTION(BlueprintCallable)
    FString GetPlayerName() const { return CurrentPlayerName; }

    // Đánh dấu host cho slot
    UFUNCTION(BlueprintCallable)
    void SetIsHost(bool bInIsHost);

    // Cập nhật ping (ms)
    UFUNCTION(BlueprintCallable)
    void SetPing(float InPing);

public:
    // Index của slot (0..N-1), có thể được set từ widget quản lý
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Slot")
    int32 SlotIndex = -1;

    UPROPERTY()
    class UInviteFriendsWidget* InviteWidget;
protected:
    // --- UI components (bind từ UMG) ---
    // Tên người chơi / trạng thái / avatar
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget))
    UImage* PlayerAvatar;

    // Border xung quanh slot (đổi màu theo ready/host/empty)
    UPROPERTY(meta = (BindWidget))
    UBorder* SlotBorder;

    // Hiển thị ping (text). Nếu muốn dùng progress bar thì đã khai báo sẵn
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PingText;

    // Tùy chọn: progress bar hiển thị chất lượng kết nối (không bắt buộc)
    UPROPERTY(meta = (BindWidget))
    UProgressBar* PingBar;

private:
    // Trạng thái runtime
    UPROPERTY()
    bool bIsOccupied = false;

    UPROPERTY()
    bool bIsReady = false;

    UPROPERTY()
    bool bIsHost = false;

    UPROPERTY()
    FString CurrentPlayerName;

    UPROPERTY()
    float Ping = 0.0f;

    // Cập nhật UI nội bộ
    void UpdateSlotContent();
};