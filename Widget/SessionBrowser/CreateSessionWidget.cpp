#include "CreateSessionWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

UCreateSessionWidget::UCreateSessionWidget(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    // Trong constructor nếu muốn khởi tạo biến mặc định, nhưng phần này UUserWidget
    // thường load các sub-widget sau khi NativeConstruct được gọi
}

void UCreateSessionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // ============ Khởi tạo dữ liệu cho ComboBox =============
    if (ComboBox_MaxPlayers)
    {
        ComboBox_MaxPlayers->ClearOptions();
        // Giả sử bạn chỉ cho phép 2 hoặc 4 người chơi
        ComboBox_MaxPlayers->AddOption(TEXT("2"));
        ComboBox_MaxPlayers->AddOption(TEXT("4"));
        ComboBox_MaxPlayers->AddOption(TEXT("8"));
        // Mặc định chọn 4
        ComboBox_MaxPlayers->SetSelectedOption(TEXT("4"));
    }

    if (ComboBox_MapSelection)
    {
        ComboBox_MapSelection->ClearOptions();
        // Thêm vào danh sách các map bạn hỗ trợ. Ví dụ 2 map:
        ComboBox_MapSelection->AddOption(TEXT("ArenaMap"));
        ComboBox_MapSelection->AddOption(TEXT("ForestMap"));
        ComboBox_MapSelection->AddOption(TEXT("DesertMap"));
        // Mặc định chọn map đầu (ví dụ "ArenaMap")
        ComboBox_MapSelection->SetSelectedOption(TEXT("ArenaMap"));
    }

    // ============ Bind sự kiện các nút =============
    Button_Create->OnClicked.AddDynamic(this, &UCreateSessionWidget::HandleOnCreateClicked);
    Button_Cancel->OnClicked.AddDynamic(this, &UCreateSessionWidget::HandleOnCancelClicked);
	Button_Back->OnClicked.AddDynamic(this, &UCreateSessionWidget::OnBackCreateSession);

    Text_ErrorMessage->SetText(FText::FromString(TEXT("")));
    Text_ErrorMessage->SetVisibility(ESlateVisibility::Collapsed);
}

bool UCreateSessionWidget::ValidateInput(FSessionSettingsData& OutData)
{
    bool bValid = true;
    FString ErrorMsg;

    // 1) RoomName không được rỗng
    if (!TextBox_RoomName)
    {
        bValid = false;
        ErrorMsg = TEXT("Error: TextBox_RoomName not bound!");
    }
    else if (TextBox_RoomName->GetText().ToString().TrimStartAndEnd().IsEmpty())
    {
        bValid = false;
        ErrorMsg = TEXT("Room name cannot be empty.");
    }
    else
    {
        OutData.RoomName = TextBox_RoomName->GetText().ToString();
    }

    // 2) Password: có thể rỗng (nếu cần), nhưng ta không ép
    if (TextBox_Password)
    {
        OutData.Password = TextBox_Password->GetText().ToString();
    }

    // 3) MaxPlayers phải được chọn từ ComboBox (2,4,8)
    if (ComboBox_MaxPlayers)
    {
        FString Sel = ComboBox_MaxPlayers->GetSelectedOption();
        int32 Num = ConvertComboBoxToInt(Sel);
        if (Num <= 0)
        {
            bValid = false;
            ErrorMsg = TEXT("Invalid Max Players value.");
        }
        else
        {
            OutData.MaxPlayers = Num;
        }
    }
    else
    {
        bValid = false;
        ErrorMsg = TEXT("Error: ComboBox_MaxPlayers not bound!");
    }

    // 4) MapSelection phải có giá trị
    if (ComboBox_MapSelection)
    {
        FString SelMap = ComboBox_MapSelection->GetSelectedOption();
        if (SelMap.IsEmpty())
        {
            bValid = false;
            ErrorMsg = TEXT("Please select a map.");
        }
        else
        {
            OutData.MapName = SelMap;
        }
    }
    else
    {
        bValid = false;
        ErrorMsg = TEXT("Error: ComboBox_MapSelection not bound!");
    }

    // Hiện thông báo lỗi nếu có
    if (!bValid && Text_ErrorMessage)
    {
        Text_ErrorMessage->SetText(FText::FromString(ErrorMsg));
        Text_ErrorMessage->SetVisibility(ESlateVisibility::Visible);
    }
    else if (bValid && Text_ErrorMessage)
    {
        Text_ErrorMessage->SetText(FText::FromString(TEXT("")));
        Text_ErrorMessage->SetVisibility(ESlateVisibility::Collapsed);
    }

    return bValid;
}

int32 UCreateSessionWidget::ConvertComboBoxToInt(const FString& InString) const
{
    return FCString::Atoi(*InString);
}

void UCreateSessionWidget::HandleOnCreateClicked()
{
    FSessionSettingsData Data;
    if (!ValidateInput(Data))
    {
        // Đã hiển thông báo lỗi trong ValidateInput()
        return;
    }

    // Đẩy delegate OnCreateSessionRequest để parent (UGameModeSelection) lắng nghe
    OnCreateSessionRequest.Broadcast(Data);
}

void UCreateSessionWidget::HandleOnCancelClicked()
{
    this->RemoveFromParent();
}

void UCreateSessionWidget::OnBackCreateSession()
{
    this->RemoveFromParent();
}
