#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorGame/Data/SessionSettingsData.h"
#include "SessionRowWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * Delegate báo parent (ServerBrowserWidget) khi user nhấn Join.
 * Truyền index của session trong danh sách.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionRowClicked, int32, RowIndex);

UCLASS()
class HORRORGAME_API USessionRowWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Thiết lập row với dữ liệu session */
    UFUNCTION()
    void Setup(const FSessionSettingsData& InData, int32 InIndex);

    /** Delegate khi user nhấn Join */
    UPROPERTY(BlueprintAssignable, Category = "SessionRow")
    FOnJoinSessionRowClicked OnJoinSessionClicked;

protected:
    UPROPERTY(meta = (BindWidget)) UTextBlock* RoomNameText;
    UPROPERTY(meta = (BindWidget)) UTextBlock* MapNameText;
    UPROPERTY(meta = (BindWidget)) UTextBlock* PlayerCountText;
    UPROPERTY(meta = (BindWidget)) UButton* JoinButton;

    int32 RowIndex;

    virtual void NativeConstruct() override;
    UFUNCTION() void OnJoinButtonClicked();
};
