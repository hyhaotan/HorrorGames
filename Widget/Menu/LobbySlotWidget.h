#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystemTypes.h"
#include "LobbySlotWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteClickedSignature, int32, SlotIndex);

class UTextBlock;
class UButton;
class UImage;

/**
 * One lobby slot: nếu có người, hiển thị tên và avatar; nếu trống, hiển thị nút Invite.
 */
UCLASS()
class HORRORGAME_API ULobbySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Gọi khi người chơi bấm Invite trên slot còn trống */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnInviteClickedSignature OnInviteClicked;

	/**
	 * Khởi tạo slot:
	 * @param InSlotIndex  – index [0..3]
	 * @param bIsOccupied  – true nếu đã có người
	 * @param DisplayName  – tên hiển thị (nếu occupied)
	 * @param InUniqueId   – ID của player (nếu occupied)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void InitializeSlot(int32 InSlotIndex, bool bIsOccupied, const FString& DisplayName = TEXT(""), const FUniqueNetIdRepl& InUniqueId = FUniqueNetIdRepl());

protected:
	/** TextBlock hiển thị tên người chơi khi occupied */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	/** Button Invite khi slot trống */
	UPROPERTY(meta = (BindWidget))
	UButton* InviteButton;

	/** Image hiện avatar hoặc icon khi occupied (nếu bạn cần) */
	UPROPERTY(meta = (BindWidget))
	UImage* PlayerAvatarImage;

private:
	int32 SlotIndex = -1;
	bool bOccupied = false;
	FUniqueNetIdRepl UniqueId;

	UFUNCTION()
	void HandleInviteButtonClicked();
};
