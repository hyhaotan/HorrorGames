// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Settings/GraphicsWidget.h"
#include "GameFramework/GameUserSettings.h"
#include <Kismet/KismetSystemLibrary.h>
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "SelectionWidget.h"
#include "HorrorGame/Widget/Framerate.h"
#include <Kismet/GameplayStatics.h>

namespace
{
	constexpr EFramrate FramerateOptions[] =
	{	EFramrate::FPS_30,
		EFramrate::FPS_60,
		EFramrate::FPS_90,
		EFramrate::FPS_120,
		EFramrate::FPS_UnCapped
	};

	typedef int32(UGameUserSettings::*GetFunc)() const;
	typedef void(UGameUserSettings::*SetFunc)(int32);

	struct FSelectionElement
	{
		USelectionWidget* Widget;
		GetFunc GetFunc;
		SetFunc SetFunc;
	};
}

void UGraphicsWidget::NativeConstruct()
{
	GameUserSettings = UGameUserSettings::GetGameUserSettings();

	InitializeResolutionComboBox();
	InitializeVSync();
	InitializeFramerate();

	const FSelectionElement SelectionElements[] = {
		{ShadingQualitySelection,&UGameUserSettings::GetShadingQuality,&UGameUserSettings::SetShadingQuality},
		{GlobalIlluminationQualitySelection,&UGameUserSettings::GetGlobalIlluminationQuality,&UGameUserSettings::SetGlobalIlluminationQuality},
		{PostProcessingQualitySelection,&UGameUserSettings::GetPostProcessingQuality,&UGameUserSettings::SetPostProcessingQuality},
		{VisualEffectsQualitySelection,&UGameUserSettings::GetVisualEffectQuality,&UGameUserSettings::SetVisualEffectQuality},
		{ShadowQualitySelection,&UGameUserSettings::GetShadowQuality,&UGameUserSettings::SetShadowQuality},
	};

	for (const auto& [Widget, GetFunc, SetFunc] : SelectionElements)
	{
		const auto CurrentSelection = std::invoke(GetFunc, GameUserSettings);
		Widget->SetCurrentSelection(CurrentSelection);
		Widget->OnSelectionChange.BindLambda([this, SetFunc](int InSelection)
			{
				std::invoke(SetFunc, GameUserSettings, InSelection);
				GameUserSettings->ApplySettings(false);
			});
	}
}

UWidget* UGraphicsWidget::NativeGetDesiredFocusTarget() const
{
	return ResolutionComboBox;
}

void UGraphicsWidget::InitializeResolutionComboBox()
{
	Resolutions.Reset();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	// Set Resolution options
	ResolutionComboBox->ClearOptions();
	for (const auto& Resolution : Resolutions)
	{
		// 1920 x 1080
		const auto ResolutionString = FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
		ResolutionComboBox->AddOption(ResolutionString);
	}

	// find the current resolution
	const auto CurrentResolution = GameUserSettings->GetScreenResolution();
	const auto SelectedIndex = Resolutions.IndexOfByPredicate([&CurrentResolution](const FIntPoint& InResolution)
		{
			return InResolution == CurrentResolution;
		});
	check(SelectedIndex >= 0);
	ResolutionComboBox->SetSelectedIndex(SelectedIndex);

	//listen to changes
	ResolutionComboBox->OnSelectionChanged.Clear();
	ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &UGraphicsWidget::OnResolutionChanged);
	
}

void UGraphicsWidget::InitializeVSync()
{
	VSyncCheckBox->SetIsChecked(GameUserSettings->IsVSyncEnabled());
	VSyncCheckBox->OnCheckStateChanged.Clear();
	VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnVSyncChanged);
}

void UGraphicsWidget::InitializeFramerate()
{
	FramerateSelection->Clear();

	int FramerateOptionIndex = 0;

	const auto CurrentFramerate = GameUserSettings->GetFrameRateLimit();
	for (const auto& Framerate : FramerateOptions)
	{
		FramerateSelection->AddOption({
			FText::FromString(FFramerateUtils::EnumToString(Framerate))
			});

		if (CurrentFramerate == FFramerateUtils::EnumToValue(Framerate))
		{
			FramerateSelection->SetCurrentSelection(FramerateOptionIndex);
		}

		FramerateOptionIndex++;
	}

	FramerateSelection->OnSelectionChange.BindLambda([this](const int InSelection)
		{
			GameUserSettings->SetFrameRateLimit(FFramerateUtils::EnumToValue(
				FramerateOptions[InSelection]
			));
			GameUserSettings->ApplySettings(false);
		});
}

void UGraphicsWidget::OnVSyncChanged(bool InIsChecked)
{
	GameUserSettings->SetVSyncEnabled(InIsChecked);
	GameUserSettings->ApplySettings(false);
}

void UGraphicsWidget::OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType)
{
	const auto SelectedResolution = Resolutions[ResolutionComboBox->GetSelectedIndex()];
	GameUserSettings->SetScreenResolution(SelectedResolution);
	GameUserSettings->ApplySettings(false);
}
