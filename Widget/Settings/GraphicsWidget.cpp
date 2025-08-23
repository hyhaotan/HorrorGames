// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorGame/Widget/Settings/GraphicsWidget.h"
#include "GameFramework/GameUserSettings.h"
#include <Kismet/KismetSystemLibrary.h>
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "SelectionWidget.h"
#include "HorrorGame/Widget/Framerate.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"
#include <Kismet/GameplayStatics.h>
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "HAL/IConsoleManager.h"

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

    if (MotionBlurCheckBox)
    {
        // bind trước rồi set trạng thái
        MotionBlurCheckBox->OnCheckStateChanged.Clear();
        MotionBlurCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnMotionBlurChanged);

        IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"));
        const int32 MotionBlurValue = (CVar != nullptr) ? CVar->GetInt() : 0;
        MotionBlurCheckBox->SetIsChecked(MotionBlurValue > 0);
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

void UGraphicsWidget::InitializeAdvancedGraphicsOptions()
{
    // Texture Quality
    const FSelectionElement TextureQualityElement = {
        TextureQualitySelection,
        &UGameUserSettings::GetTextureQuality,
        &UGameUserSettings::SetTextureQuality
    };

    const auto CurrentTextureQuality = std::invoke(TextureQualityElement.GetFunc, GameUserSettings);
    TextureQualitySelection->SetCurrentSelection(CurrentTextureQuality);
    TextureQualitySelection->OnSelectionChange.BindLambda([this, TextureQualityElement](int InSelection)
        {
            std::invoke(TextureQualityElement.SetFunc, GameUserSettings, InSelection);
            GameUserSettings->ApplySettings(false);
        });

    // View Distance Quality
    const FSelectionElement ViewDistanceQualityElement = {
        ViewDistanceQualitySelection,
        &UGameUserSettings::GetViewDistanceQuality,
        &UGameUserSettings::SetViewDistanceQuality
    };

    const auto CurrentViewDistanceQuality = std::invoke(ViewDistanceQualityElement.GetFunc, GameUserSettings);
    ViewDistanceQualitySelection->SetCurrentSelection(CurrentViewDistanceQuality);
    ViewDistanceQualitySelection->OnSelectionChange.BindLambda([this, ViewDistanceQualityElement](int InSelection)
        {
            std::invoke(ViewDistanceQualityElement.SetFunc, GameUserSettings, InSelection);
            GameUserSettings->ApplySettings(false);
        });

    // Anti-Aliasing Quality
    const FSelectionElement AntiAliasingQualityElement = {
        AntiAliasingQualitySelection,
        &UGameUserSettings::GetAntiAliasingQuality,
        &UGameUserSettings::SetAntiAliasingQuality
    };

    const auto CurrentAntiAliasingQuality = std::invoke(AntiAliasingQualityElement.GetFunc, GameUserSettings);
    AntiAliasingQualitySelection->SetCurrentSelection(CurrentAntiAliasingQuality);
    AntiAliasingQualitySelection->OnSelectionChange.BindLambda([this, AntiAliasingQualityElement](int InSelection)
        {
            std::invoke(AntiAliasingQualityElement.SetFunc, GameUserSettings, InSelection);
            GameUserSettings->ApplySettings(false);
        });

    // Reflection Quality
    const FSelectionElement ReflectionQualityElement = {
        ReflectionQualitySelection,
        &UGameUserSettings::GetReflectionQuality,
        &UGameUserSettings::SetReflectionQuality
    };

    const auto CurrentReflectionQuality = std::invoke(ReflectionQualityElement.GetFunc, GameUserSettings);
    ReflectionQualitySelection->SetCurrentSelection(CurrentReflectionQuality);
    ReflectionQualitySelection->OnSelectionChange.BindLambda([this, ReflectionQualityElement](int InSelection)
        {
            std::invoke(ReflectionQualityElement.SetFunc, GameUserSettings, InSelection);
            GameUserSettings->ApplySettings(false);
        });

    // Foliage Quality
    const FSelectionElement FoliageQualityElement = {
        FoliageQualitySelection,
        &UGameUserSettings::GetFoliageQuality,
        &UGameUserSettings::SetFoliageQuality
    };

    const auto CurrentFoliageQuality = std::invoke(FoliageQualityElement.GetFunc, GameUserSettings);
    FoliageQualitySelection->SetCurrentSelection(CurrentFoliageQuality);
    FoliageQualitySelection->OnSelectionChange.BindLambda([this, FoliageQualityElement](int InSelection)
        {
            std::invoke(FoliageQualityElement.SetFunc, GameUserSettings, InSelection);
            GameUserSettings->ApplySettings(false);
        });

    if (MotionBlurCheckBox)
    {
        if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
        {
            MotionBlurCheckBox->SetIsChecked(Settings->GetPostProcessingQuality() > 0);
        }
        else
        {
            MotionBlurCheckBox->SetIsChecked(false);
        }

        MotionBlurCheckBox->OnCheckStateChanged.Clear();
        MotionBlurCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnMotionBlurChanged);
    }

    // Field of View
    if (FieldOfViewSlider && FieldOfViewText)
    {
        // Get FOV from HorrorGameSettings since UE doesn't have a built-in FOV setting
        if (UHorrorGameSettings* HorrorSettings = UHorrorGameSettings::Get())
        {
            // Assuming you add FOV to HorrorGameSettings
            const float CurrentFOV = 90.0f; // Default FOV, get from custom settings
            FieldOfViewSlider->SetValue(CurrentFOV);
            FieldOfViewText->SetText(FText::AsNumber(CurrentFOV));
            FieldOfViewSlider->OnValueChanged.Clear();
            FieldOfViewSlider->OnValueChanged.AddDynamic(this, &UGraphicsWidget::OnFieldOfViewChanged);
        }
    }
}

void UGraphicsWidget::InitializeDisplayOptions()
{
    // Fullscreen Mode Selection
    if (FullscreenModeSelection)
    {
        FullscreenModeSelection->Clear();

        const FString FullscreenModes[] = {
            TEXT("Windowed"),
            TEXT("Windowed Fullscreen"),
            TEXT("Fullscreen")
        };

        const EWindowMode::Type CurrentWindowMode = GameUserSettings->GetFullscreenMode();

        for (int32 i = 0; i < 3; ++i)
        {
            FullscreenModeSelection->AddOption({
                FText::FromString(FullscreenModes[i])
                });

            if ((i == 0 && CurrentWindowMode == EWindowMode::Windowed) ||
                (i == 1 && CurrentWindowMode == EWindowMode::WindowedFullscreen) ||
                (i == 2 && CurrentWindowMode == EWindowMode::Fullscreen))
            {
                FullscreenModeSelection->SetCurrentSelection(i);
            }
        }

        FullscreenModeSelection->OnSelectionChange.BindLambda([this](int32 InSelection)
            {
                EWindowMode::Type NewWindowMode = EWindowMode::Windowed;
                switch (InSelection)
                {
                case 0: NewWindowMode = EWindowMode::Windowed; break;
                case 1: NewWindowMode = EWindowMode::WindowedFullscreen; break;
                case 2: NewWindowMode = EWindowMode::Fullscreen; break;
                }

                GameUserSettings->SetFullscreenMode(NewWindowMode);
                GameUserSettings->ApplySettings(false);
            });
    }
}

void UGraphicsWidget::OnMotionBlurChanged(bool bIsChecked)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"));
    if (CVar)
    {
        CVar->Set(bIsChecked ? 3 : 0); // set quality 3 (max) hoặc 0 (off)
    }

    // Tùy ý: gọi ApplySettings() để reapply user settings nếu muốn
    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    {
        Settings->ApplySettings(false);
    }
}

void UGraphicsWidget::OnBloomChanged(bool bIsChecked)
{
    // Implement bloom setting through console variable or custom setting
    // TODO: Implement bloom toggle
}

void UGraphicsWidget::OnLensFlareChanged(bool bIsChecked)
{
    // Implement lens flare setting through console variable or custom setting
    // TODO: Implement lens flare toggle
}

void UGraphicsWidget::OnFieldOfViewChanged(float Value)
{
    if (FieldOfViewText)
    {
        FieldOfViewText->SetText(FText::AsNumber(Value));
    }

    // Apply FOV change to player camera
    // TODO: Implement FOV application to player camera
}

void UGraphicsWidget::OnBenchmarkClicked()
{
    // Run graphics benchmark
    // TODO: Implement automatic graphics quality detection based on performance
}