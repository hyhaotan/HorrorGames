#include "GraphicsWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "HAL/IConsoleManager.h"
#include "HorrorGame/Widget/Settings/SelectionWidget.h"
#include "HorrorGame/Widget/Framerate.h"
#include "HorrorGame/Settings/HorrorGameSettings.h"

namespace GraphicsWidgetPrivate
{
	// Framerate options available in the game
	constexpr EFramrate FramerateOptions[] =
	{
		EFramrate::FPS_30,
		EFramrate::FPS_60,
		EFramrate::FPS_90,
		EFramrate::FPS_120,
		EFramrate::FPS_UnCapped
	};

	// Quality setting function pointers for easy iteration
	typedef int32(UGameUserSettings::* GetQualityFunc)() const;
	typedef void(UGameUserSettings::* SetQualityFunc)(int32);

	struct FQualitySettingInfo
	{
		USelectionWidget* Widget;
		GetQualityFunc GetFunc;
		SetQualityFunc SetFunc;
		FString Name; // For logging purposes
	};

	// Console variable names for advanced graphics options
	static const FString CVarMotionBlur = TEXT("r.MotionBlurQuality");
	static const FString CVarBloom = TEXT("r.BloomQuality");
	static const FString CVarLensFlare = TEXT("r.LensFlareQuality");
	static const FString CVarSSAO = TEXT("r.AmbientOcclusionLevels");
	static const FString CVarScreenSpaceReflections = TEXT("r.SSR.Quality");

	// Default quality values
	static constexpr int32 DefaultMotionBlurQuality = 3;
	static constexpr int32 DefaultBloomQuality = 5;
	static constexpr int32 DefaultLensFlareQuality = 2;
	static constexpr float DefaultFOV = 90.0f;
}

void UGraphicsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize game user settings
	GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("GraphicsWidget: Failed to get GameUserSettings"));
		return;
	}

	// Validate all required UI components
	if (!ValidateGraphicsComponents())
	{
		UE_LOG(LogTemp, Error, TEXT("GraphicsWidget: Component validation failed"));
		return;
	}

	// Initialize all graphics settings
	InitializeResolutionComboBox();
	InitializeVSync();
	InitializeFramerate();
	InitializeQualitySettings();
	InitializeAdvancedGraphicsOptions();
	InitializeDisplayOptions();

	UE_LOG(LogTemp, Log, TEXT("GraphicsWidget: Successfully initialized"));
}

UWidget* UGraphicsWidget::NativeGetDesiredFocusTarget() const
{
	return ResolutionComboBox ? ResolutionComboBox : nullptr;
}

void UGraphicsWidget::InitializeResolutionComboBox()
{
	if (!ResolutionComboBox || !GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeResolutionComboBox: Missing required components"));
		return;
	}

	// Clear existing data
	Resolutions.Reset();
	ResolutionComboBox->ClearOptions();

	// Get supported resolutions
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	if (Resolutions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeResolutionComboBox: No supported resolutions found"));
		return;
	}

	// Filter and add valid resolutions
	for (int32 i = 0; i < Resolutions.Num(); ++i)
	{
		const FIntPoint& Resolution = Resolutions[i];

		// Validate resolution values
		if (Resolution.X > 0 && Resolution.Y > 0 && Resolution.X >= 800 && Resolution.Y >= 600)
		{
			const FString ResolutionString = FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
			ResolutionComboBox->AddOption(ResolutionString);
		}
		else
		{
			// Remove invalid resolutions from array
			Resolutions.RemoveAt(i);
			--i;
		}
	}

	// Set current resolution
	const FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
	UE_LOG(LogTemp, Log, TEXT("Current Resolution: %dx%d"), CurrentResolution.X, CurrentResolution.Y);

	// Find and set the matching resolution
	const int32 SelectedIndex = Resolutions.IndexOfByPredicate([&CurrentResolution](const FIntPoint& InResolution)
		{
			return InResolution == CurrentResolution;
		});

	if (SelectedIndex != INDEX_NONE)
	{
		ResolutionComboBox->SetSelectedIndex(SelectedIndex);
		UE_LOG(LogTemp, Log, TEXT("Resolution set to index: %d"), SelectedIndex);
	}
	else
	{
		// Find closest resolution as fallback
		const int32 FallbackIndex = FindClosestResolutionIndex(CurrentResolution);
		if (FallbackIndex != INDEX_NONE)
		{
			ResolutionComboBox->SetSelectedIndex(FallbackIndex);
			UE_LOG(LogTemp, Warning, TEXT("Using closest resolution match at index: %d"), FallbackIndex);
		}
		else if (Resolutions.Num() > 0)
		{
			ResolutionComboBox->SetSelectedIndex(0);
			UE_LOG(LogTemp, Warning, TEXT("Using first available resolution as fallback"));
		}
	}

	// Bind resolution change event
	ResolutionComboBox->OnSelectionChanged.Clear();
	ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &UGraphicsWidget::OnResolutionChanged);
}

void UGraphicsWidget::InitializeVSync()
{
	if (!VSyncCheckBox || !GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeVSync: Missing required components"));
		return;
	}

	// Set VSync state
	const bool bVSyncEnabled = GameUserSettings->IsVSyncEnabled();
	VSyncCheckBox->SetIsChecked(bVSyncEnabled);

	UE_LOG(LogTemp, Log, TEXT("VSync initialized: %s"), bVSyncEnabled ? TEXT("Enabled") : TEXT("Disabled"));

	// Bind VSync change event
	VSyncCheckBox->OnCheckStateChanged.Clear();
	VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnVSyncChanged);
}

void UGraphicsWidget::InitializeFramerate()
{
	if (!FramerateSelection || !GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeFramerate: Missing required components"));
		return;
	}

	FramerateSelection->Clear();

	int32 FramerateOptionIndex = 0;
	int32 CurrentFramerateIndex = 0;
	const float CurrentFramerate = GameUserSettings->GetFrameRateLimit();

	// Add framerate options
	for (const auto& Framerate : GraphicsWidgetPrivate::FramerateOptions)
	{
		const FString FramerateString = FFramerateUtils::EnumToString(Framerate);
		FramerateSelection->AddOption({ FText::FromString(FramerateString) });

		// Check if this matches current framerate
		if (FMath::IsNearlyEqual(CurrentFramerate, FFramerateUtils::EnumToValue(Framerate), 0.1f))
		{
			CurrentFramerateIndex = FramerateOptionIndex;
		}

		FramerateOptionIndex++;
	}

	// Set current selection
	FramerateSelection->SetCurrentSelection(CurrentFramerateIndex);

	// Bind framerate change event
	FramerateSelection->OnSelectionChange.BindLambda([this](const int32 InSelection)
		{
			if (InSelection >= 0 && InSelection < UE_ARRAY_COUNT(GraphicsWidgetPrivate::FramerateOptions))
			{
				const float NewFrameRate = FFramerateUtils::EnumToValue(GraphicsWidgetPrivate::FramerateOptions[InSelection]);
				GameUserSettings->SetFrameRateLimit(NewFrameRate);
				GameUserSettings->ApplySettings(false);

				UE_LOG(LogTemp, Log, TEXT("Framerate changed to: %.1f"), NewFrameRate);
			}
		});
}

void UGraphicsWidget::InitializeQualitySettings()
{
	if (!GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeQualitySettings: GameUserSettings is null"));
		return;
	}

	// Define quality setting mappings
	const GraphicsWidgetPrivate::FQualitySettingInfo QualitySettings[] = {
		{ShadingQualitySelection, &UGameUserSettings::GetShadingQuality, &UGameUserSettings::SetShadingQuality, TEXT("Shading")},
		{GlobalIlluminationQualitySelection, &UGameUserSettings::GetGlobalIlluminationQuality, &UGameUserSettings::SetGlobalIlluminationQuality, TEXT("GlobalIllumination")},
		{PostProcessingQualitySelection, &UGameUserSettings::GetPostProcessingQuality, &UGameUserSettings::SetPostProcessingQuality, TEXT("PostProcessing")},
		{VisualEffectsQualitySelection, &UGameUserSettings::GetVisualEffectQuality, &UGameUserSettings::SetVisualEffectQuality, TEXT("VisualEffects")},
		{ShadowQualitySelection, &UGameUserSettings::GetShadowQuality, &UGameUserSettings::SetShadowQuality, TEXT("Shadow")},
		{TextureQualitySelection, &UGameUserSettings::GetTextureQuality, &UGameUserSettings::SetTextureQuality, TEXT("Texture")},
		{ViewDistanceQualitySelection, &UGameUserSettings::GetViewDistanceQuality, &UGameUserSettings::SetViewDistanceQuality, TEXT("ViewDistance")},
		{AntiAliasingQualitySelection, &UGameUserSettings::GetAntiAliasingQuality, &UGameUserSettings::SetAntiAliasingQuality, TEXT("AntiAliasing")},
		{ReflectionQualitySelection, &UGameUserSettings::GetReflectionQuality, &UGameUserSettings::SetReflectionQuality, TEXT("Reflection")},
		{FoliageQualitySelection, &UGameUserSettings::GetFoliageQuality, &UGameUserSettings::SetFoliageQuality, TEXT("Foliage")}
	};

	// Initialize each quality setting
	for (const auto& QualitySetting : QualitySettings)
	{
		if (!QualitySetting.Widget)
		{
			UE_LOG(LogTemp, Warning, TEXT("InitializeQualitySettings: %s widget is null"), *QualitySetting.Name);
			continue;
		}

		// Get current quality value
		const int32 CurrentQuality = std::invoke(QualitySetting.GetFunc, GameUserSettings);
		QualitySetting.Widget->SetCurrentSelection(CurrentQuality);

		// Bind quality change event
		QualitySetting.Widget->OnSelectionChange.BindLambda([this, QualitySetting](int32 InSelection)
			{
				std::invoke(QualitySetting.SetFunc, GameUserSettings, InSelection);
				GameUserSettings->ApplySettings(false);

				UE_LOG(LogTemp, Log, TEXT("%s quality changed to: %d"), *QualitySetting.Name, InSelection);
			});

		UE_LOG(LogTemp, Log, TEXT("%s quality initialized: %d"), *QualitySetting.Name, CurrentQuality);
	}
}

void UGraphicsWidget::InitializeAdvancedGraphicsOptions()
{
	// Initialize Motion Blur
	if (MotionBlurCheckBox)
	{
		IConsoleVariable* MotionBlurCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarMotionBlur);
		const int32 MotionBlurValue = MotionBlurCVar ? MotionBlurCVar->GetInt() : 0;

		MotionBlurCheckBox->SetIsChecked(MotionBlurValue > 0);
		MotionBlurCheckBox->OnCheckStateChanged.Clear();
		MotionBlurCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnMotionBlurChanged);

		UE_LOG(LogTemp, Log, TEXT("Motion Blur initialized: %s"), MotionBlurValue > 0 ? TEXT("Enabled") : TEXT("Disabled"));
	}

	// Initialize Bloom
	if (BloomCheckBox)
	{
		IConsoleVariable* BloomCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarBloom);
		const int32 BloomValue = BloomCVar ? BloomCVar->GetInt() : GraphicsWidgetPrivate::DefaultBloomQuality;

		BloomCheckBox->SetIsChecked(BloomValue > 0);
		BloomCheckBox->OnCheckStateChanged.Clear();
		BloomCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnBloomChanged);

		UE_LOG(LogTemp, Log, TEXT("Bloom initialized: %s"), BloomValue > 0 ? TEXT("Enabled") : TEXT("Disabled"));
	}

	// Initialize Lens Flare
	if (LensFlareCheckBox)
	{
		IConsoleVariable* LensFlareCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarLensFlare);
		const int32 LensFlareValue = LensFlareCVar ? LensFlareCVar->GetInt() : GraphicsWidgetPrivate::DefaultLensFlareQuality;

		LensFlareCheckBox->SetIsChecked(LensFlareValue > 0);
		LensFlareCheckBox->OnCheckStateChanged.Clear();
		LensFlareCheckBox->OnCheckStateChanged.AddDynamic(this, &UGraphicsWidget::OnLensFlareChanged);

		UE_LOG(LogTemp, Log, TEXT("Lens Flare initialized: %s"), LensFlareValue > 0 ? TEXT("Enabled") : TEXT("Disabled"));
	}

	// Initialize Field of View
	InitializeFieldOfView();
}

void UGraphicsWidget::InitializeDisplayOptions()
{
	if (!FullscreenModeSelection || !GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeDisplayOptions: Missing required components"));
		return;
	}

	FullscreenModeSelection->Clear();

	// Define fullscreen mode options
	const FString FullscreenModes[] = {
		TEXT("Windowed"),
		TEXT("Windowed Fullscreen"),
		TEXT("Fullscreen")
	};

	const EWindowMode::Type CurrentWindowMode = GameUserSettings->GetFullscreenMode();
	int32 CurrentModeIndex = 0;

	// Add fullscreen mode options and find current selection
	for (int32 i = 0; i < UE_ARRAY_COUNT(FullscreenModes); ++i)
	{
		FullscreenModeSelection->AddOption({ FText::FromString(FullscreenModes[i]) });

		// Determine current mode index
		if ((i == 0 && CurrentWindowMode == EWindowMode::Windowed) ||
			(i == 1 && CurrentWindowMode == EWindowMode::WindowedFullscreen) ||
			(i == 2 && CurrentWindowMode == EWindowMode::Fullscreen))
		{
			CurrentModeIndex = i;
		}
	}

	// Set current selection
	FullscreenModeSelection->SetCurrentSelection(CurrentModeIndex);

	// Bind fullscreen mode change event
	FullscreenModeSelection->OnSelectionChange.BindLambda([this](int32 InSelection)
		{
			EWindowMode::Type NewWindowMode = EWindowMode::Windowed;
			FString ModeName = TEXT("Windowed");

			switch (InSelection)
			{
			case 0:
				NewWindowMode = EWindowMode::Windowed;
				ModeName = TEXT("Windowed");
				break;
			case 1:
				NewWindowMode = EWindowMode::WindowedFullscreen;
				ModeName = TEXT("Windowed Fullscreen");
				break;
			case 2:
				NewWindowMode = EWindowMode::Fullscreen;
				ModeName = TEXT("Fullscreen");
				break;
			default:
				UE_LOG(LogTemp, Warning, TEXT("Invalid fullscreen mode selection: %d"), InSelection);
				return;
			}

			GameUserSettings->SetFullscreenMode(NewWindowMode);
			GameUserSettings->ApplySettings(false);

			UE_LOG(LogTemp, Log, TEXT("Fullscreen mode changed to: %s"), *ModeName);
		});

	// Initialize buttons
	InitializeActionButtons();

	UE_LOG(LogTemp, Log, TEXT("Display options initialized"));
}

void UGraphicsWidget::InitializeFieldOfView()
{
	if (!FieldOfViewSlider || !FieldOfViewText)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFieldOfView: FOV widgets not available"));
		return;
	}

	// Get current FOV from horror game settings
	float CurrentFOV = GraphicsWidgetPrivate::DefaultFOV;
	if (UHorrorGameSettings* HorrorSettings = UHorrorGameSettings::Get())
	{
		CurrentFOV = HorrorSettings->GetFieldOfView();
	}

	// Set slider properties
	FieldOfViewSlider->SetMinValue(60.0f);
	FieldOfViewSlider->SetMaxValue(120.0f);
	FieldOfViewSlider->SetStepSize(1.0f);
	FieldOfViewSlider->SetValue(CurrentFOV);

	// Set text display
	FieldOfViewText->SetText(FText::AsNumber(FMath::RoundToInt(CurrentFOV)));

	// Bind FOV change event
	FieldOfViewSlider->OnValueChanged.Clear();
	FieldOfViewSlider->OnValueChanged.AddDynamic(this, &UGraphicsWidget::OnFieldOfViewChanged);

	UE_LOG(LogTemp, Log, TEXT("Field of View initialized: %.1f"), CurrentFOV);
}

void UGraphicsWidget::InitializeActionButtons()
{
	// Initialize Benchmark Button
	if (BenchmarkButton)
	{
		BenchmarkButton->OnClicked.Clear();
		BenchmarkButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnBenchmarkClicked);
	}

	// Initialize Auto Detect Button
	if (AutoDetectButton)
	{
		AutoDetectButton->OnClicked.Clear();
		AutoDetectButton->OnClicked.AddDynamic(this, &UGraphicsWidget::OnAutoDetectClicked);
	}
}

void UGraphicsWidget::OnVSyncChanged(bool InIsChecked)
{
	if (!GameUserSettings)
		return;

	GameUserSettings->SetVSyncEnabled(InIsChecked);
	GameUserSettings->ApplySettings(false);

	UE_LOG(LogTemp, Log, TEXT("VSync %s"), InIsChecked ? TEXT("Enabled") : TEXT("Disabled"));
}

void UGraphicsWidget::OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType)
{
	if (!ResolutionComboBox || !GameUserSettings)
		return;

	const int32 SelectedIndex = ResolutionComboBox->GetSelectedIndex();
	if (!Resolutions.IsValidIndex(SelectedIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("OnResolutionChanged: Invalid resolution index %d"), SelectedIndex);
		return;
	}

	const FIntPoint& SelectedResolution = Resolutions[SelectedIndex];
	GameUserSettings->SetScreenResolution(SelectedResolution);
	GameUserSettings->ApplySettings(false);

	UE_LOG(LogTemp, Log, TEXT("Resolution changed to: %dx%d"), SelectedResolution.X, SelectedResolution.Y);
}

void UGraphicsWidget::OnMotionBlurChanged(bool bIsChecked)
{
	IConsoleVariable* MotionBlurCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarMotionBlur);
	if (MotionBlurCVar)
	{
		const int32 NewValue = bIsChecked ? GraphicsWidgetPrivate::DefaultMotionBlurQuality : 0;
		MotionBlurCVar->Set(NewValue);

		UE_LOG(LogTemp, Log, TEXT("Motion Blur %s (Quality: %d)"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"), NewValue);
	}

	if (GameUserSettings)
	{
		GameUserSettings->ApplySettings(false);
	}
}

void UGraphicsWidget::OnBloomChanged(bool bIsChecked)
{
	IConsoleVariable* BloomCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarBloom);
	if (BloomCVar)
	{
		const int32 NewValue = bIsChecked ? GraphicsWidgetPrivate::DefaultBloomQuality : 0;
		BloomCVar->Set(NewValue);

		UE_LOG(LogTemp, Log, TEXT("Bloom %s (Quality: %d)"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"), NewValue);
	}

	if (GameUserSettings)
	{
		GameUserSettings->ApplySettings(false);
	}
}

void UGraphicsWidget::OnLensFlareChanged(bool bIsChecked)
{
	IConsoleVariable* LensFlareCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarLensFlare);
	if (LensFlareCVar)
	{
		const int32 NewValue = bIsChecked ? GraphicsWidgetPrivate::DefaultLensFlareQuality : 0;
		LensFlareCVar->Set(NewValue);

		UE_LOG(LogTemp, Log, TEXT("Lens Flare %s (Quality: %d)"), bIsChecked ? TEXT("Enabled") : TEXT("Disabled"), NewValue);
	}

	if (GameUserSettings)
	{
		GameUserSettings->ApplySettings(false);
	}
}

void UGraphicsWidget::OnFieldOfViewChanged(float Value)
{
	const int32 RoundedValue = FMath::RoundToInt(Value);

	// Update text display
	if (FieldOfViewText)
	{
		FieldOfViewText->SetText(FText::AsNumber(RoundedValue));
	}

	// Save FOV to horror game settings
	if (UHorrorGameSettings* HorrorSettings = UHorrorGameSettings::Get())
	{
		HorrorSettings->SetFieldOfView(Value);
		HorrorSettings->SaveConfig();

		// Broadcast FOV change event
		HorrorSettings->OnFOVChanged.Broadcast(Value);

		UE_LOG(LogTemp, Log, TEXT("Field of View changed to: %.1f"), Value);
	}
}

void UGraphicsWidget::OnBenchmarkClicked()
{
	if (!GameUserSettings)
		return;

	UE_LOG(LogTemp, Log, TEXT("Starting graphics benchmark..."));

	// Run hardware benchmark
	GameUserSettings->RunHardwareBenchmark();

	// Apply benchmark results automatically
	GameUserSettings->ApplyHardwareBenchmarkResults();

	// Refresh UI to show new settings
	RefreshValues();

	UE_LOG(LogTemp, Log, TEXT("Graphics benchmark completed and applied"));
}

void UGraphicsWidget::OnAutoDetectClicked()
{
	if (!GameUserSettings)
		return;

	UE_LOG(LogTemp, Log, TEXT("Auto-detecting optimal graphics settings..."));

	// Run hardware benchmark for auto-detection
	GameUserSettings->RunHardwareBenchmark();
	GameUserSettings->ApplyHardwareBenchmarkResults();

	// Set additional optimizations for horror game
	OptimizeForHorrorGame();

	// Apply settings
	GameUserSettings->ApplySettings(false);

	// Refresh UI
	RefreshValues();

	UE_LOG(LogTemp, Log, TEXT("Auto-detection completed"));
}

void UGraphicsWidget::RefreshValues()
{
	if (!GameUserSettings)
	{
		GameUserSettings = UGameUserSettings::GetGameUserSettings();
		if (!GameUserSettings)
		{
			UE_LOG(LogTemp, Error, TEXT("RefreshValues: Failed to get GameUserSettings"));
			return;
		}
	}

	// Refresh basic settings
	InitializeResolutionComboBox();
	InitializeVSync();
	InitializeFramerate();

	// Refresh quality settings
	InitializeQualitySettings();

	// Refresh advanced options
	InitializeAdvancedGraphicsOptions();

	// Refresh display options
	InitializeDisplayOptions();

	UE_LOG(LogTemp, Log, TEXT("Graphics settings refreshed"));
}

void UGraphicsWidget::ResetToDefaults()
{
	if (!GameUserSettings)
	{
		GameUserSettings = UGameUserSettings::GetGameUserSettings();
		if (!GameUserSettings)
		{
			UE_LOG(LogTemp, Error, TEXT("ResetToDefaults: Failed to get GameUserSettings"));
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Resetting graphics settings to defaults..."));

	// Reset UE settings to system defaults
	GameUserSettings->SetToDefaults();

	// Set recommended defaults for horror game
	SetHorrorGameDefaults();

	// Reset console variables
	ResetConsoleVariables();

	// Reset custom horror game settings
	if (UHorrorGameSettings* HorrorSettings = UHorrorGameSettings::Get())
	{
		HorrorSettings->SetFieldOfView(GraphicsWidgetPrivate::DefaultFOV);
		HorrorSettings->SaveConfig();
	}

	// Apply all settings
	GameUserSettings->ApplySettings(false);

	// Refresh UI
	RefreshValues();

	UE_LOG(LogTemp, Log, TEXT("Graphics settings reset to defaults completed"));
}

void UGraphicsWidget::SetHorrorGameDefaults()
{
	if (!GameUserSettings)
		return;

	// Set quality defaults optimized for horror games
	GameUserSettings->SetShadingQuality(2);           // Medium
	GameUserSettings->SetShadowQuality(3);            // High (important for horror atmosphere)
	GameUserSettings->SetPostProcessingQuality(3);    // High (important for horror effects)
	GameUserSettings->SetVisualEffectQuality(2);      // Medium
	GameUserSettings->SetTextureQuality(2);           // Medium
	GameUserSettings->SetAntiAliasingQuality(2);      // Medium
	GameUserSettings->SetViewDistanceQuality(2);      // Medium
	GameUserSettings->SetReflectionQuality(1);        // Low (performance optimization)
	GameUserSettings->SetFoliageQuality(2);           // Medium
	GameUserSettings->SetGlobalIlluminationQuality(2); // Medium

	// Set performance defaults
	GameUserSettings->SetVSyncEnabled(false);         // Disable for better performance
	GameUserSettings->SetFrameRateLimit(60.0f);       // Standard 60 FPS
	GameUserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen); // Best compatibility

	UE_LOG(LogTemp, Log, TEXT("Horror game default settings applied"));
}

void UGraphicsWidget::ResetConsoleVariables()
{
	// Reset advanced graphics console variables
	if (IConsoleVariable* MotionBlurCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarMotionBlur))
	{
		MotionBlurCVar->Set(GraphicsWidgetPrivate::DefaultMotionBlurQuality);
	}

	if (IConsoleVariable* BloomCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarBloom))
	{
		BloomCVar->Set(GraphicsWidgetPrivate::DefaultBloomQuality);
	}

	if (IConsoleVariable* LensFlareCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarLensFlare))
	{
		LensFlareCVar->Set(GraphicsWidgetPrivate::DefaultLensFlareQuality);
	}

	UE_LOG(LogTemp, Log, TEXT("Console variables reset to defaults"));
}

void UGraphicsWidget::OptimizeForHorrorGame()
{
	if (!GameUserSettings)
		return;

	// Get current overall quality for reference
	const int32 OverallQuality = GameUserSettings->GetOverallScalabilityLevel();

	// Optimize specific settings for horror atmosphere
	// Shadows are crucial for horror atmosphere
	GameUserSettings->SetShadowQuality(FMath::Max(2, OverallQuality));

	// Post-processing important for horror effects
	GameUserSettings->SetPostProcessingQuality(FMath::Max(2, OverallQuality));

	// Reduce reflection quality for performance (not as important for horror)
	GameUserSettings->SetReflectionQuality(FMath::Max(0, OverallQuality - 1));

	UE_LOG(LogTemp, Log, TEXT("Applied horror game optimizations based on quality level: %d"), OverallQuality);
}

int32 UGraphicsWidget::FindClosestResolutionIndex(const FIntPoint& TargetResolution)
{
	if (Resolutions.Num() == 0)
	{
		return INDEX_NONE;
	}

	int32 ClosestIndex = 0;
	float SmallestDistance = FLT_MAX;

	for (int32 i = 0; i < Resolutions.Num(); ++i)
	{
		const FIntPoint& Resolution = Resolutions[i];

		// Calculate distance using euclidean distance
		const float DiffX = static_cast<float>(Resolution.X - TargetResolution.X);
		const float DiffY = static_cast<float>(Resolution.Y - TargetResolution.Y);
		const float Distance = FMath::Sqrt(DiffX * DiffX + DiffY * DiffY);

		if (Distance < SmallestDistance)
		{
			SmallestDistance = Distance;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
}

bool UGraphicsWidget::ValidateGraphicsComponents()
{
	bool bAllValid = true;
	TArray<FString> MissingComponents;

	// Check core components
	if (!ResolutionComboBox)
	{
		MissingComponents.Add(TEXT("ResolutionComboBox"));
		bAllValid = false;
	}

	if (!VSyncCheckBox)
	{
		MissingComponents.Add(TEXT("VSyncCheckBox"));
		bAllValid = false;
	}

	if (!FramerateSelection)
	{
		MissingComponents.Add(TEXT("FramerateSelection"));
		bAllValid = false;
	}

	// Define struct for widget validation
	struct FWidgetValidation
	{
		USelectionWidget* Widget;
		FString Name;
	};

	// Check quality selection widgets using simple struct
	const FWidgetValidation QualityWidgets[] = {
		{ShadingQualitySelection, TEXT("ShadingQualitySelection")},
		{GlobalIlluminationQualitySelection, TEXT("GlobalIlluminationQualitySelection")},
		{PostProcessingQualitySelection, TEXT("PostProcessingQualitySelection")},
		{VisualEffectsQualitySelection, TEXT("VisualEffectsQualitySelection")},
		{ShadowQualitySelection, TEXT("ShadowQualitySelection")},
		{TextureQualitySelection, TEXT("TextureQualitySelection")},
		{ViewDistanceQualitySelection, TEXT("ViewDistanceQualitySelection")},
		{AntiAliasingQualitySelection, TEXT("AntiAliasingQualitySelection")},
		{ReflectionQualitySelection, TEXT("ReflectionQualitySelection")},
		{FoliageQualitySelection, TEXT("FoliageQualitySelection")}
	};

	for (const FWidgetValidation& WidgetInfo : QualityWidgets)
	{
		if (!WidgetInfo.Widget)
		{
			MissingComponents.Add(WidgetInfo.Name);
			// Don't mark as invalid for quality widgets - they might be optional
			UE_LOG(LogTemp, Warning, TEXT("Optional quality widget missing: %s"), *WidgetInfo.Name);
		}
	}

	// Check GameUserSettings
	if (!GameUserSettings)
	{
		MissingComponents.Add(TEXT("GameUserSettings"));
		bAllValid = false;
	}

	// Log results
	if (bAllValid)
	{
		UE_LOG(LogTemp, Log, TEXT("GraphicsWidget component validation passed"));
	}
	else
	{
		FString MissingList = FString::Join(MissingComponents, TEXT(", "));
		UE_LOG(LogTemp, Error, TEXT("GraphicsWidget validation failed. Missing components: %s"), *MissingList);
	}

	return bAllValid;
}

void UGraphicsWidget::ApplySettingsWithValidation()
{
	if (!GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("ApplySettingsWithValidation: GameUserSettings is null"));
		return;
	}

	// Validate settings before applying
	const FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
	if (CurrentResolution.X <= 0 || CurrentResolution.Y <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid resolution detected, resetting to safe default"));
		GameUserSettings->SetScreenResolution(FIntPoint(1920, 1080));
	}

	// Apply settings
	GameUserSettings->ApplySettings(false);

	// Confirm settings were applied
	GameUserSettings->ConfirmVideoMode();

	UE_LOG(LogTemp, Log, TEXT("Graphics settings applied and confirmed"));
}

void UGraphicsWidget::SaveUserSettings()
{
	if (!GameUserSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveUserSettings: GameUserSettings is null"));
		return;
	}

	// Save UE graphics settings
	GameUserSettings->SaveSettings();

	// Save custom horror game settings
	if (UHorrorGameSettings* HorrorSettings = UHorrorGameSettings::Get())
	{
		HorrorSettings->SaveConfig();
	}

	UE_LOG(LogTemp, Log, TEXT("User graphics settings saved"));
}

void UGraphicsWidget::LoadUserSettings()
{
	if (!GameUserSettings)
	{
		GameUserSettings = UGameUserSettings::GetGameUserSettings();
		if (!GameUserSettings)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadUserSettings: Failed to get GameUserSettings"));
			return;
		}
	}

	// Load settings
	GameUserSettings->LoadSettings();

	// Refresh UI to reflect loaded settings
	RefreshValues();

	UE_LOG(LogTemp, Log, TEXT("User graphics settings loaded"));
}

FString UGraphicsWidget::GetQualityLevelName(int32 QualityLevel)
{
	switch (QualityLevel)
	{
	case 0: return TEXT("Low");
	case 1: return TEXT("Medium");
	case 2: return TEXT("High");
	case 3: return TEXT("Epic");
	case 4: return TEXT("Cinematic");
	default: return TEXT("Custom");
	}
}

void UGraphicsWidget::LogCurrentSettings()
{
	if (!GameUserSettings)
		return;

	UE_LOG(LogTemp, Log, TEXT("=== Current Graphics Settings ==="));

	// Basic settings
	const FIntPoint Resolution = GameUserSettings->GetScreenResolution();
	UE_LOG(LogTemp, Log, TEXT("Resolution: %dx%d"), Resolution.X, Resolution.Y);
	UE_LOG(LogTemp, Log, TEXT("VSync: %s"), GameUserSettings->IsVSyncEnabled() ? TEXT("On") : TEXT("Off"));
	UE_LOG(LogTemp, Log, TEXT("Frame Rate Limit: %.1f"), GameUserSettings->GetFrameRateLimit());

	// Quality settings
	UE_LOG(LogTemp, Log, TEXT("Overall Quality: %s"), *GetQualityLevelName(GameUserSettings->GetOverallScalabilityLevel()));
	UE_LOG(LogTemp, Log, TEXT("Shading Quality: %s"), *GetQualityLevelName(GameUserSettings->GetShadingQuality()));
	UE_LOG(LogTemp, Log, TEXT("Shadow Quality: %s"), *GetQualityLevelName(GameUserSettings->GetShadowQuality()));
	UE_LOG(LogTemp, Log, TEXT("Post Processing Quality: %s"), *GetQualityLevelName(GameUserSettings->GetPostProcessingQuality()));
	UE_LOG(LogTemp, Log, TEXT("Texture Quality: %s"), *GetQualityLevelName(GameUserSettings->GetTextureQuality()));
	UE_LOG(LogTemp, Log, TEXT("Effects Quality: %s"), *GetQualityLevelName(GameUserSettings->GetVisualEffectQuality()));
	UE_LOG(LogTemp, Log, TEXT("Foliage Quality: %s"), *GetQualityLevelName(GameUserSettings->GetFoliageQuality()));
	UE_LOG(LogTemp, Log, TEXT("Anti-Aliasing Quality: %s"), *GetQualityLevelName(GameUserSettings->GetAntiAliasingQuality()));
	UE_LOG(LogTemp, Log, TEXT("View Distance Quality: %s"), *GetQualityLevelName(GameUserSettings->GetViewDistanceQuality()));

	// Window mode
	FString WindowModeStr;
	switch (GameUserSettings->GetFullscreenMode())
	{
	case EWindowMode::Windowed: WindowModeStr = TEXT("Windowed"); break;
	case EWindowMode::WindowedFullscreen: WindowModeStr = TEXT("Windowed Fullscreen"); break;
	case EWindowMode::Fullscreen: WindowModeStr = TEXT("Fullscreen"); break;
	default: WindowModeStr = TEXT("Unknown"); break;
	}
	UE_LOG(LogTemp, Log, TEXT("Window Mode: %s"), *WindowModeStr);

	// Advanced settings
	if (IConsoleVariable* MotionBlurCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarMotionBlur))
	{
		UE_LOG(LogTemp, Log, TEXT("Motion Blur Quality: %d"), MotionBlurCVar->GetInt());
	}

	if (IConsoleVariable* BloomCVar = IConsoleManager::Get().FindConsoleVariable(*GraphicsWidgetPrivate::CVarBloom))
	{
		UE_LOG(LogTemp, Log, TEXT("Bloom Quality: %d"), BloomCVar->GetInt());
	}

	// Custom settings
	if (UHorrorGameSettings* HorrorSettings = UHorrorGameSettings::Get())
	{
		UE_LOG(LogTemp, Log, TEXT("Field of View: %.1f"), HorrorSettings->GetFieldOfView());
	}

	UE_LOG(LogTemp, Log, TEXT("=== End Graphics Settings ==="));
}

void UGraphicsWidget::SetPresetQuality(int32 QualityPreset)
{
	if (!GameUserSettings)
		return;

	// Clamp quality preset to valid range
	QualityPreset = FMath::Clamp(QualityPreset, 0, 4);

	UE_LOG(LogTemp, Log, TEXT("Applying quality preset: %s"), *GetQualityLevelName(QualityPreset));

	// Apply preset to all quality settings
	GameUserSettings->SetOverallScalabilityLevel(QualityPreset);

	// For horror games, ensure shadows and post-processing maintain minimum quality
	if (QualityPreset == 0) // Low preset
	{
		GameUserSettings->SetShadowQuality(1);          // Keep shadows at least medium-low
		GameUserSettings->SetPostProcessingQuality(1);  // Keep post-processing at least medium-low
	}

	// Apply settings
	GameUserSettings->ApplySettings(false);

	// Refresh UI
	RefreshValues();

	UE_LOG(LogTemp, Log, TEXT("Quality preset applied: %s"), *GetQualityLevelName(QualityPreset));
}

bool UGraphicsWidget::IsResolutionSupported(const FIntPoint& Resolution)
{
	for (const FIntPoint& SupportedResolution : Resolutions)
	{
		if (SupportedResolution == Resolution)
		{
			return true;
		}
	}
	return false;
}

void UGraphicsWidget::RefreshResolutionList()
{
	// Clear current resolutions
	Resolutions.Reset();

	// Get fresh list of supported resolutions
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	// Re-initialize resolution combo box
	InitializeResolutionComboBox();

	UE_LOG(LogTemp, Log, TEXT("Resolution list refreshed - Found %d supported resolutions"), Resolutions.Num());
}

void UGraphicsWidget::OnWindowModeChanged()
{
	// This function can be called when window mode changes externally
	// Refresh the display options to reflect current state
	InitializeDisplayOptions();
}

TArray<FIntPoint> UGraphicsWidget::GetSupportedResolutions() const
{
	return Resolutions;
}