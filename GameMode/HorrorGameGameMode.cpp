#include "HorrorGameGameMode.h"
#include "HorrorGame/HorrorGameCharacter.h"
#include "HorrorGame/Actor/EyeMonster.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AHorrorGameGameMode::AHorrorGameGameMode()
{
    // Set default pawn class to our Blueprinted character.
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
}
