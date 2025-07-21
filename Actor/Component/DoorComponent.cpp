// DoorComponent.cpp

#include "DoorComponent.h"
#include "Components/TimelineComponent.h"

UDoorComponent::UDoorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
	bHasOpened = false;
}

void UDoorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (DoorOpenCurve)
	{
		FOnTimelineFloat ProgressCallback;
		ProgressCallback.BindUFunction(this, FName("HandleDoorProgress"));
		DoorTimeline->AddInterpFloat(DoorOpenCurve, ProgressCallback);
	}
}

void UDoorComponent::HandleDoorProgress(float Value)
{
	if (DoorPivot)
	{
		FRotator NewRotation = FMath::Lerp(ClosedRotation, OpenRotation, Value);
		DoorPivot->SetRelativeRotation(NewRotation);
	}
}

void UDoorComponent::OpenDoor()
{
	if (bHasOpened || !DoorTimeline) return;
	bHasOpened = true;
	DoorTimeline->PlayFromStart();
}
