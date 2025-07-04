#include "CreateSessionWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OnlineSessionSettings.h"
#include "ServerBrowserWidget.h"

void UCreateSessionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ComboBox_MaxPlayers)
    {
        ComboBox_MaxPlayers->ClearOptions();
        ComboBox_MaxPlayers->AddOption(TEXT("2"));
        ComboBox_MaxPlayers->AddOption(TEXT("4"));
        ComboBox_MaxPlayers->AddOption(TEXT("8"));
        ComboBox_MaxPlayers->SetSelectedOption(TEXT("4"));
    }
    if (ComboBox_MapSelection)
    {
        ComboBox_MapSelection->ClearOptions();
        ComboBox_MapSelection->AddOption(TEXT("L_horrorGame"));
        ComboBox_MapSelection->AddOption(TEXT("ForestMap"));
        ComboBox_MapSelection->AddOption(TEXT("DesertMap"));
        ComboBox_MapSelection->SetSelectedOption(TEXT("L_horrorGame"));
    }

    if (Button_Create) Button_Create->OnClicked.AddDynamic(this, &UCreateSessionWidget::HandleCreateClicked);
    if (Button_Cancel) Button_Cancel->OnClicked.AddDynamic(this, &UCreateSessionWidget::HandleCancelClicked);

    if (Text_ErrorMessage)
    {
        Text_ErrorMessage->SetText(FText());
        Text_ErrorMessage->SetVisibility(ESlateVisibility::Collapsed);
    }
}

bool UCreateSessionWidget::ValidateInput(FSessionSettingsData& OutData)
{
    if (!TextBox_RoomName || TextBox_RoomName->GetText().ToString().TrimStartAndEnd().IsEmpty())
    {
        if (Text_ErrorMessage)
        {
            Text_ErrorMessage->SetText(FText::FromString(TEXT("Room name cannot be empty.")));
            Text_ErrorMessage->SetVisibility(ESlateVisibility::Visible);
        }
        return false;
    }
    OutData.RoomName = TextBox_RoomName->GetText().ToString();
    OutData.Password = TextBox_Password ? TextBox_Password->GetText().ToString() : TEXT("");
    OutData.MaxPlayers = ConvertStringToInt(ComboBox_MaxPlayers->GetSelectedOption());
    OutData.MapName = ComboBox_MapSelection->GetSelectedOption();
    if (Text_ErrorMessage)
    {
        Text_ErrorMessage->SetText(FText());
        Text_ErrorMessage->SetVisibility(ESlateVisibility::Collapsed);
    }
    return true;
}

int32 UCreateSessionWidget::ConvertStringToInt(const FString& In) const { return FCString::Atoi(*In); }

void UCreateSessionWidget::HandleCreateClicked()
{
    FSessionSettingsData Data;
    if (!ValidateInput(Data)) return;
    CreateHostSession(Data);
}

void UCreateSessionWidget::HandleCancelClicked()
{
    RemoveFromParent();
}

void UCreateSessionWidget::CreateHostSession(const FSessionSettingsData& Data)
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(FName("Steam"));
    if (!OSS) return;
    IOnlineSessionPtr Sessions = OSS->GetSessionInterface();
    if (!Sessions.IsValid()) return;

    Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
    CreateSessionCompleteHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UCreateSessionWidget::OnCreateSessionComplete)
    );

    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = Data.MaxPlayers;
    Settings.bIsLANMatch = false;
    Settings.bUsesPresence = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bShouldAdvertise = true;
    Settings.Set(FName(TEXT("ROOM_NAME")), Data.RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
    Settings.Set(FName(TEXT("PASSWORD")), Data.Password, EOnlineDataAdvertisementType::ViaOnlineService);
    Settings.Set(FName(TEXT("MAP_NAME")), Data.MapName, EOnlineDataAdvertisementType::ViaOnlineService);

    Sessions->CreateSession(0, UServerBrowserWidget::Name_GameSession, Settings);
}

void UCreateSessionWidget::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete: %s – Success=%d"),
        *InSessionName.ToString(), bWasSuccessful);

    // Xoá delegate ngay
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    auto Sessions = OSS->GetSessionInterface();
    if (OSS)
    {
        if (Sessions.IsValid())
            Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
    }

    if (!bWasSuccessful)
    {
        if (Text_ErrorMessage)
        {
            Text_ErrorMessage->SetText(FText::FromString(TEXT("Failed to create session.")));
            Text_ErrorMessage->SetVisibility(ESlateVisibility::Visible);
        }
        return;
    }

    // Nếu thành công, tiếp tục StartSession (tuỳ chọn) rồi OpenLevel
    if (Sessions.IsValid())
    {
        Sessions->StartSession(InSessionName);
        UE_LOG(LogTemp, Warning, TEXT("StartSession called on %s"), *InSessionName.ToString());
    }
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("LobbyMap"), true, TEXT("?listen"));
}

