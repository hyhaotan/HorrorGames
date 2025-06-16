#include "FriendListEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UFriendListEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Btn_Invite)
    {
        Btn_Invite->OnClicked.AddDynamic(this, &UFriendListEntryWidget::OnInviteButtonPressed);
    }
}

void UFriendListEntryWidget::Setup(UFriendEntryData* InData, FOnInviteClicked InDelegate)
{
    FriendData = InData;
    InviteDelegate = MoveTemp(InDelegate);

    if (Text_FriendName && FriendData)
    {
        Text_FriendName->SetText(FText::FromString(FriendData->DisplayName));
    }
}

void UFriendListEntryWidget::OnInviteButtonPressed()
{
    if (InviteDelegate.IsBound() && FriendData)
    {
        InviteDelegate.Execute(FriendData);
    }
}
