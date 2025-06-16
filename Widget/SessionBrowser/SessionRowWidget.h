#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "SessionRowWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * Delegate để báo parent (ServerBrowserWidget) biết user bấm nút Join của row này.
 * Truyền vào chỉ số (Index) của session trong mảng tìm kiếm.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionRowClicked, int32, RowIndex);

UCLASS()
class HORRORGAME_API USessionRowWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Gọi để setup row với dữ liệu tìm được và chỉ số trong mảng SearchResults */
    void SetupRow(const FOnlineSessionSearchResult& InSearchResult, int32 InIndex);

    /** Delegate khi user bấm nút Join trên row này */
    UPROPERTY(BlueprintAssignable, Category = "SessionRow")
    FOnJoinSessionRowClicked OnJoinSessionClicked;

protected:
    /** TextBlock hiển thị Room Name */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* RoomNameText;

    /** TextBlock hiển thị Map Name */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MapNameText;

    /** TextBlock hiển thị Player Count (ví dụ "2 / 4") */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerCountText;

    /** Button để Join vào session này */
    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton;

    /** Lưu chỉ số của row để broadcast lên parent */
    int32 RowIndex;

    // Override để bind event cho JoinButton
    virtual void NativeConstruct() override;

    /** Hàm gọi khi nhấn JoinButton */
    UFUNCTION()
    void OnJoinButtonClicked();
};
