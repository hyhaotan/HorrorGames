#pragma once

UENUM(BlueprintType)
enum class EFramrate : uint8
{
	FPS_30 UMETA(DisplayName = "30 FPS"),
	FPS_60 UMETA(DisplayName = "60 FPS"),
	FPS_90 UMETA(DisplayName = "90 FPS"),
	FPS_120 UMETA(DisplayName = "120 FPS"),
	FPS_UnCapped UMETA(DisplayName = "UnCapped"),
};

class FFramerateUtils
{
public:
	static int EnumToValue(const EFramrate& InFramerate)
	{
		switch (InFramerate)
		{
		case EFramrate::FPS_30:return 30;
		case EFramrate::FPS_60:return 60;
		case EFramrate::FPS_90:return 90;
		case EFramrate::FPS_120:return 120;
		default:return 0;
		}
	}

	static FString EnumToString(const EFramrate& InFramerate)
	{
		const auto Value = EnumToValue(InFramerate);
		return Value > 0
			?FString::Printf(TEXT("%d FPS"), Value)
			:FString::Printf(TEXT("UnCapped"));

	}
};