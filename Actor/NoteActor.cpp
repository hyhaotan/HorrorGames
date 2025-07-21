#include "HorrorGame/Actor/NoteActor.h"
#include "HorrorGame/Character/HorrorGameCharacter.h"
#include "HorrorGame/Widget/Item/ItemWidget.h"
#include "HorrorGame/HUD/PlayerHUDWidget.h"
#include "HorrorGame/Widget/MainHUDWidget.h"
#include "HorrorGame/Actor/Door/ElectronicLockActor.h"
#include "HorrorGame/Widget/Item/NoteWidget.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"

ANoteActor::ANoteActor()
{
}

void ANoteActor::BeginPlay()
{
	Super::BeginPlay();

	ElectronicLockActor = Cast<AElectronicLockActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ElectronicLockActorClass));
    if (!ElectronicLockActor)
    {
            UE_LOG(LogTemp, Warning, TEXT("ElectronicLockActor not found in the world!"));
			return;
    }
	OriginalTransform = GetActorTransform();

    ElectronicLockActor->CorrectCode.Empty();
    for (int32 i = 0; i < CodeLength; ++i)
    {
        ElectronicLockActor->CorrectCode.Add(FMath::RandRange(0, 9));
    }

    CodeString.Empty();
    for (int32 D : ElectronicLockActor->CorrectCode)
        CodeString.AppendInt(D);

    DigitStrings.Empty();
    for (int32 Index = 0; Index < CodeString.Len(); ++Index)
        {
            DigitStrings.Add(CodeString.Mid(Index, 1));
        }

    // Broadcast so UI hint shows
    ElectronicLockActor->OnCodeUpdated.Broadcast(ElectronicLockActor->CorrectCode);
}

void ANoteActor::Interact(AHorrorGameCharacter* Player)
{
    if (!Player) return;

    TogglePlayerHUD(false);
    Player->SetCurrentNoteActor(this);
    HideNote();

    if (!bNoteShown && NoteTexture)
    {
        // Lấy từng chữ số từ DigitStrings
        const FString& D1 = DigitStrings.IsValidIndex(0) ? DigitStrings[0] : TEXT("?");
        const FString& D2 = DigitStrings.IsValidIndex(1) ? DigitStrings[1] : TEXT("?");
        const FString& D3 = DigitStrings.IsValidIndex(2) ? DigitStrings[2] : TEXT("?");
        const FString& D4 = DigitStrings.IsValidIndex(3) ? DigitStrings[3] : TEXT("?");

        FString Story = FString::Printf(
            TEXT(
                "1. The Shattered Window\n"
                "You step into the abandoned farmhouse, the wind rattling a broken window with a hollow clang.\n"
                "On the sill, someone has scrawled a single character in what looks like dried blood: %s.\n"
                "The number drips down into the dust below, as if beckoning you further inside.\n\n"

                "2. The Whispered Chant\n"
                "Through the narrow hallway, a soft voice hisses behind peeling wallpaper.\n"
                "Leaning closer, you see four tiny scratches carved into the wood—only one is still visible: %s.\n"
                "The others have faded, but this one shines fresh and bright.\n\n"

                "3. The Mirror’s Reflection\n"
                "In the parlor stands an ornate mirror, its glass fractured into jagged shards.\n"
                "As you approach, your own face warps, and in the upper corner you catch a flash of white chalk: %s.\n"
                "It’s etched upside‐down, as though someone on the other side tried to warn you.\n\n"

                "4. The Final Door\n"
                "At the end of the corridor looms a tall iron door, its lock blinking with an otherworldly glow.\n"
                "Below the keypad, four carved indentations cradle faint numerals—only one remains clear: %s.\n"
                "The others have been worn by time and terror.\n\n"

                "Your Task: Assemble the four digits in the order you discovered them to unlock the door’s seal.\n"
                "Good luck… if you dare."
            ),
            *D1, *D2, *D3, *D4
        );

        UNoteWidget* NoteW = Player->ShowNoteUI(NoteTexture, FText::GetEmpty());
        if (NoteW)
        {
            NoteW->SetupNoteText(FText::FromString(Story));
        }

        bNoteShown = true;
        return;
    }
}

void ANoteActor::HideNote()
{
	// ẩn mesh + tắt collision
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ANoteActor::ReturnToOriginal()
{
	TogglePlayerHUD(true);

	// trả về vị trí gốc, hiện lại và bật collision
	SetActorTransform(OriginalTransform);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ANoteActor::TogglePlayerHUD(bool bVisible)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Lấy HUD và gọi đến MainWidget để set visibility
    APlayerHUDWidget* PlayerHUD = Cast<APlayerHUDWidget>(PC->GetHUD());
    if (PlayerHUD && PlayerHUD->MainWidgetInstance)
    {
        PlayerHUD->MainWidgetInstance->SetAllPanelsVisibility(bVisible);
    }

	AHorrorGameCharacter* Player = Cast<AHorrorGameCharacter>(PC->GetPawn());
	if (Player)
	{
		Player->SetInventoryVisible(bVisible); 
	}
}
