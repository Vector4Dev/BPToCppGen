#include "SBPToCppGenPanel.h"
#include "BPDescriptionParser.h"
#include "BlueprintAssetReader.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"

void SBPToCppGenPanel::Construct(const FArguments& InArgs)
{
	StatusMessage = TEXT("Paste a Blueprint description and click Generate.");
	StatusSeverity = EBPStatusSeverity::Info;

	static const FString SampleText =
		TEXT("Class: BP_EnemyController\nParent: AAIController\n\nVariables:\n  Health (float) = 100.0\n  IsAlerted (bool) = false\n  TargetActor (AActor*)\n\nFunctions:\n  TakeDamage(Amount: float) -> void\n  FindNearestTarget() -> AActor*\n\nEvents:\n  BeginPlay (override)\n  Tick (override, DeltaTime: float)\n");

	const FCheckBoxStyle* ToggleStyle = &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckbox");

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 8.f, 8.f, 4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Input Source")))
				.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.Style(ToggleStyle)
				.IsChecked(this, &SBPToCppGenPanel::IsSourcePasteChecked)
				.OnCheckStateChanged(this, &SBPToCppGenPanel::OnSourcePasteChanged)
				.Padding(FMargin(14.f, 5.f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Paste Text")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.Style(ToggleStyle)
				.IsChecked(this, &SBPToCppGenPanel::IsSourceBlueprintChecked)
				.OnCheckStateChanged(this, &SBPToCppGenPanel::OnSourceBlueprintChanged)
				.Padding(FMargin(14.f, 5.f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Blueprint Asset")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 12.f, 0.f)
			[
				SNew(SCheckBox)
				.Style(ToggleStyle)
				.IsChecked(this, &SBPToCppGenPanel::IsSourceVerseChecked)
				.OnCheckStateChanged(this, &SBPToCppGenPanel::OnSourceVerseChanged)
				.Padding(FMargin(14.f, 5.f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Verse File")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SAssignNew(BlueprintPickerButton, SComboButton)
				.Visibility(this, &SBPToCppGenPanel::GetBlueprintPickerVisibility)
				.ButtonContent()
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Choose Blueprint...")))
				]
				.OnGetMenuContent(this, &SBPToCppGenPanel::BuildBlueprintPickerMenu)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SButton)
				.Visibility(this, &SBPToCppGenPanel::GetVerseImportVisibility)
				.Text(FText::FromString(TEXT("Choose .verse File...")))
				.OnClicked(this, &SBPToCppGenPanel::OnImportVerseClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Clear")))
				.OnClicked(this, &SBPToCppGenPanel::OnClearClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 0.f, 8.f, 4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Output Target")))
				.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.Style(ToggleStyle)
				.IsChecked(this, &SBPToCppGenPanel::IsCppTargetChecked)
				.OnCheckStateChanged(this, &SBPToCppGenPanel::OnCppTargetChanged)
				.Padding(FMargin(14.f, 5.f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("C++")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.Style(ToggleStyle)
				.IsChecked(this, &SBPToCppGenPanel::IsVerseTargetChecked)
				.OnCheckStateChanged(this, &SBPToCppGenPanel::OnVerseTargetChanged)
				.Padding(FMargin(14.f, 5.f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Verse")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.Style(ToggleStyle)
				.IsChecked(this, &SBPToCppGenPanel::IsBlueprintTargetChecked)
				.OnCheckStateChanged(this, &SBPToCppGenPanel::OnBlueprintTargetChanged)
				.Padding(FMargin(14.f, 5.f))
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Blueprint Asset")))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 0.f, 8.f, 4.f)
		[
			SNew(SHorizontalBox)
			.Visibility(this, &SBPToCppGenPanel::GetApiMacroRowVisibility)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("Module API Macro:")))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SAssignNew(ApiMacroTextBox, SEditableTextBox)
				.Text(FText::FromString(TEXT("YOURGAME_API")))
				.HintText(FText::FromString(TEXT("e.g. MYGAME_API")))
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(8.f, 4.f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			+ SSplitter::Slot()
			.Value(0.4f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Blueprint Description")))
					.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(InputTextBox, SMultiLineEditableTextBox)
					.Text(FText::FromString(SampleText))
					.AllowMultiLine(true)
					.AutoWrapText(false)
				]
			]

			+ SSplitter::Slot()
			.Value(0.6f)
			[
				SAssignNew(PreviewSwitcher, SWidgetSwitcher)

				+ SWidgetSwitcher::Slot()
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 4.f)
						[
							SNew(STextBlock)
							.Text(this, &SBPToCppGenPanel::GetHeaderPaneTitleText)
							.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SNew(SMultiLineEditableTextBox)
							.IsReadOnly(true)
							.Text(this, &SBPToCppGenPanel::GetHeaderPreviewText)
						]
					]
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 4.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Source (.cpp)")))
							.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SNew(SMultiLineEditableTextBox)
							.IsReadOnly(true)
							.Text(this, &SBPToCppGenPanel::GetCppPreviewText)
						]
					]
				]

				+ SWidgetSwitcher::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(STextBlock)
						.Text(this, &SBPToCppGenPanel::GetHeaderPaneTitleText)
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SMultiLineEditableTextBox)
						.IsReadOnly(true)
						.Text(this, &SBPToCppGenPanel::GetHeaderPreviewText)
					]
				]

				+ SWidgetSwitcher::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Blueprint Asset")))
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 10))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(FText::FromString(TEXT("No text preview for this target. Set a content path below and click Create Blueprint Asset to write a real .uasset with your variables and function signatures. Event overrides are not created and must be added manually in the Blueprint editor.")))
					]
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 4.f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ContentPadding(FMargin(0.f, 8.f))
			.Text(this, &SBPToCppGenPanel::GetPrimaryActionLabelText)
			.OnClicked(this, &SBPToCppGenPanel::OnPrimaryActionClicked)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 4.f)
		[
			SNew(SHorizontalBox)
			.Visibility(this, &SBPToCppGenPanel::GetFolderExportRowVisibility)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SAssignNew(OutputFolderTextBox, SEditableTextBox)
				.HintText(FText::FromString(TEXT("Target Source folder (e.g. C:/MyProject/Source/MyGame)")))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Browse...")))
				.OnClicked(this, &SBPToCppGenPanel::OnBrowseFolderClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Write Files")))
				.OnClicked(this, &SBPToCppGenPanel::OnWriteFilesClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 4.f)
		[
			SNew(SHorizontalBox)
			.Visibility(this, &SBPToCppGenPanel::GetBlueprintExportRowVisibility)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SAssignNew(BlueprintPackagePathTextBox, SEditableTextBox)
				.Text(FText::FromString(TEXT("/Game/Blueprints")))
				.HintText(FText::FromString(TEXT("Content path (e.g. /Game/Blueprints)")))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 0.f, 8.f, 8.f)
		[
			SNew(STextBlock)
			.Text(this, &SBPToCppGenPanel::GetStatusText)
			.ColorAndOpacity(this, &SBPToCppGenPanel::GetStatusColor)
		]
	];
}

void SBPToCppGenPanel::SetStatus(const FString& Message, EBPStatusSeverity Severity)
{
	StatusMessage = Message;
	StatusSeverity = Severity;
}

ECheckBoxState SBPToCppGenPanel::IsCppTargetChecked() const
{
	return (OutputTarget == EBPOutputTarget::Cpp) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
ECheckBoxState SBPToCppGenPanel::IsVerseTargetChecked() const
{
	return (OutputTarget == EBPOutputTarget::Verse) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
ECheckBoxState SBPToCppGenPanel::IsBlueprintTargetChecked() const
{
	return (OutputTarget == EBPOutputTarget::BlueprintAsset) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SBPToCppGenPanel::OnCppTargetChanged(ECheckBoxState NewState)
{
	if (NewState == ECheckBoxState::Checked) { SetOutputTarget(EBPOutputTarget::Cpp); }
}
void SBPToCppGenPanel::OnVerseTargetChanged(ECheckBoxState NewState)
{
	if (NewState == ECheckBoxState::Checked) { SetOutputTarget(EBPOutputTarget::Verse); }
}
void SBPToCppGenPanel::OnBlueprintTargetChanged(ECheckBoxState NewState)
{
	if (NewState == ECheckBoxState::Checked) { SetOutputTarget(EBPOutputTarget::BlueprintAsset); }
}

void SBPToCppGenPanel::SetOutputTarget(EBPOutputTarget NewTarget)
{
	if (OutputTarget == NewTarget)
	{
		return;
	}

	OutputTarget = NewTarget;
	bHasGenerated = false;

	if (PreviewSwitcher.IsValid())
	{
		const int32 Index = (NewTarget == EBPOutputTarget::Cpp) ? 0 : (NewTarget == EBPOutputTarget::Verse ? 1 : 2);
		PreviewSwitcher->SetActiveWidgetIndex(Index);
	}

	switch (NewTarget)
	{
	case EBPOutputTarget::Cpp:
		SetStatus(TEXT("Switched to C++ output. Click Generate."), EBPStatusSeverity::Info);
		break;
	case EBPOutputTarget::Verse:
		SetStatus(TEXT("Switched to Verse output. Click Generate."), EBPStatusSeverity::Info);
		break;
	default:
		SetStatus(TEXT("Switched to Blueprint Asset output. Set a content path and click Create Blueprint Asset."), EBPStatusSeverity::Info);
		break;
	}
}

ECheckBoxState SBPToCppGenPanel::IsSourcePasteChecked() const
{
	return (InputSource == EBPInputSource::PasteText) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
ECheckBoxState SBPToCppGenPanel::IsSourceBlueprintChecked() const
{
	return (InputSource == EBPInputSource::BlueprintAsset) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
ECheckBoxState SBPToCppGenPanel::IsSourceVerseChecked() const
{
	return (InputSource == EBPInputSource::VerseFile) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
void SBPToCppGenPanel::OnSourcePasteChanged(ECheckBoxState NewState)
{
	if (NewState == ECheckBoxState::Checked) { SetInputSource(EBPInputSource::PasteText); }
}
void SBPToCppGenPanel::OnSourceBlueprintChanged(ECheckBoxState NewState)
{
	if (NewState == ECheckBoxState::Checked) { SetInputSource(EBPInputSource::BlueprintAsset); }
}
void SBPToCppGenPanel::OnSourceVerseChanged(ECheckBoxState NewState)
{
	if (NewState == ECheckBoxState::Checked) { SetInputSource(EBPInputSource::VerseFile); }
}

void SBPToCppGenPanel::SetInputSource(EBPInputSource NewSource)
{
	InputSource = NewSource;
	switch (NewSource)
	{
	case EBPInputSource::PasteText:
		SetStatus(TEXT("Paste a Blueprint description directly into the box below."), EBPStatusSeverity::Info);
		break;
	case EBPInputSource::BlueprintAsset:
		SetStatus(TEXT("Click 'Choose Blueprint...' to pick an asset."), EBPStatusSeverity::Info);
		break;
	default:
		SetStatus(TEXT("Click 'Choose .verse File...' to import one."), EBPStatusSeverity::Info);
		break;
	}
}

EVisibility SBPToCppGenPanel::GetApiMacroRowVisibility() const
{
	return (OutputTarget == EBPOutputTarget::Cpp) ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SBPToCppGenPanel::GetFolderExportRowVisibility() const
{
	return (OutputTarget != EBPOutputTarget::BlueprintAsset) ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SBPToCppGenPanel::GetBlueprintExportRowVisibility() const
{
	return (OutputTarget == EBPOutputTarget::BlueprintAsset) ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SBPToCppGenPanel::GetBlueprintPickerVisibility() const
{
	return (InputSource == EBPInputSource::BlueprintAsset) ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SBPToCppGenPanel::GetVerseImportVisibility() const
{
	return (InputSource == EBPInputSource::VerseFile) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SBPToCppGenPanel::GetPrimaryActionLabelText() const
{
	return (OutputTarget == EBPOutputTarget::BlueprintAsset) ? FText::FromString(TEXT("Create Blueprint Asset")) : FText::FromString(TEXT("Generate"));
}

FReply SBPToCppGenPanel::OnClearClicked()
{
	InputTextBox->SetText(FText::GetEmpty());
	bHasGenerated = false;
	SetStatus(TEXT("Cleared. Paste a Blueprint description and click Generate."), EBPStatusSeverity::Info);
	return FReply::Handled();
}

TSharedRef<SWidget> SBPToCppGenPanel::BuildBlueprintPickerMenu()
{
	FAssetPickerConfig PickerConfig;
	PickerConfig.Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	PickerConfig.Filter.bRecursiveClasses = true;
	PickerConfig.bAllowNullSelection = false;
	PickerConfig.InitialAssetViewType = EAssetViewType::List;
	PickerConfig.bFocusSearchBoxWhenOpened = true;
	PickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SBPToCppGenPanel::OnBlueprintAssetSelected);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	return SNew(SBox)
		.WidthOverride(400.f)
		.HeightOverride(400.f)
		[
			ContentBrowserModule.Get().CreateAssetPicker(PickerConfig)
		];
}

void SBPToCppGenPanel::OnBlueprintAssetSelected(const FAssetData& AssetData)
{
	FSlateApplication::Get().DismissAllMenus();

	UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
	if (!Blueprint)
	{
		SetStatus(TEXT("Selected asset is not a Blueprint."), EBPStatusSeverity::Error);
		return;
	}

	FBPClassDesc ClassDesc;
	TArray<FString> Warnings;
	const bool bRead = FBlueprintAssetReader::ReadBlueprint(Blueprint, ClassDesc, Warnings);

	if (!bRead)
	{
		SetStatus(TEXT("Failed to read Blueprint: ") + FString::Join(Warnings, TEXT(" | ")), EBPStatusSeverity::Error);
		return;
	}

	InputTextBox->SetText(FText::FromString(FBPDescriptionParser::Serialize(ClassDesc)));

	if (Warnings.Num() > 0)
	{
		SetStatus(FString::Printf(TEXT("Loaded %s with %d warning(s): %s"), *ClassDesc.ClassName, Warnings.Num(), *FString::Join(Warnings, TEXT(" | "))), EBPStatusSeverity::Warning);
	}
	else
	{
		SetStatus(FString::Printf(TEXT("Loaded %s, review then click Generate."), *ClassDesc.ClassName), EBPStatusSeverity::Success);
	}
}

FReply SBPToCppGenPanel::OnImportVerseClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FReply::Handled();
	}

	TArray<FString> OutFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const bool bOpened = DesktopPlatform->OpenFileDialog(ParentWindowHandle, TEXT("Import Verse File"), TEXT(""), TEXT(""), TEXT("Verse Files (*.verse)|*.verse"), EFileDialogFlags::None, OutFiles);

	if (!bOpened || OutFiles.Num() == 0)
	{
		return FReply::Handled();
	}

	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *OutFiles[0]))
	{
		SetStatus(TEXT("Failed to read the selected Verse file."), EBPStatusSeverity::Error);
		return FReply::Handled();
	}

	FBPClassDesc ClassDesc;
	TArray<FString> Warnings;
	const bool bRead = FVerseSourceReader::ReadVerseSource(FileContents, ClassDesc, Warnings);

	if (!bRead)
	{
		SetStatus(TEXT("Failed to parse Verse source: ") + FString::Join(Warnings, TEXT(" | ")), EBPStatusSeverity::Error);
		return FReply::Handled();
	}

	InputTextBox->SetText(FText::FromString(FBPDescriptionParser::Serialize(ClassDesc)));
	SetOutputTarget(EBPOutputTarget::Cpp);
	bHasGenerated = false;

	SetStatus(FString::Printf(TEXT("Imported %s from Verse with %d warning(s): %s"), *ClassDesc.ClassName, Warnings.Num(), *FString::Join(Warnings, TEXT(" | "))), Warnings.Num() > 0 ? EBPStatusSeverity::Warning : EBPStatusSeverity::Success);

	return FReply::Handled();
}

FReply SBPToCppGenPanel::OnPrimaryActionClicked()
{
	if (OutputTarget == EBPOutputTarget::BlueprintAsset)
	{
		RunCreateBlueprint();
	}
	else
	{
		RunGenerate();
	}
	return FReply::Handled();
}

void SBPToCppGenPanel::RunGenerate()
{
	const FString SourceText = InputTextBox->GetText().ToString();
	const FString ApiMacro = ApiMacroTextBox->GetText().ToString();

	FBPClassDesc ClassDesc;
	TArray<FString> Errors;
	const bool bParsed = FBPDescriptionParser::Parse(SourceText, ClassDesc, Errors);

	if (!bParsed)
	{
		bHasGenerated = false;
		SetStatus(TEXT("Failed to parse: ") + FString::Join(Errors, TEXT(" | ")), EBPStatusSeverity::Error);
		return;
	}

	if (OutputTarget == EBPOutputTarget::Cpp)
	{
		LastGenerated = FCppSkeletonGenerator::Generate(ClassDesc, ApiMacro);
	}
	else
	{
		TArray<FString> VerseWarnings;
		LastGeneratedVerse = FVerseSkeletonGenerator::Generate(ClassDesc, VerseWarnings);
		Errors.Append(VerseWarnings);
	}
	bHasGenerated = true;

	if (Errors.Num() > 0)
	{
		SetStatus(FString::Printf(TEXT("Generated with %d warning(s): %s"), Errors.Num(), *FString::Join(Errors, TEXT(" | "))), EBPStatusSeverity::Warning);
	}
	else if (OutputTarget == EBPOutputTarget::Cpp)
	{
		SetStatus(FString::Printf(TEXT("Generated %s / %s"), *LastGenerated.HeaderFileName, *LastGenerated.CppFileName), EBPStatusSeverity::Success);
	}
	else
	{
		SetStatus(FString::Printf(TEXT("Generated %s"), *LastGeneratedVerse.FileName), EBPStatusSeverity::Success);
	}
}

void SBPToCppGenPanel::RunCreateBlueprint()
{
	const FString SourceText = InputTextBox->GetText().ToString();

	FBPClassDesc ClassDesc;
	TArray<FString> ParseErrors;
	if (!FBPDescriptionParser::Parse(SourceText, ClassDesc, ParseErrors))
	{
		SetStatus(TEXT("Failed to parse: ") + FString::Join(ParseErrors, TEXT(" | ")), EBPStatusSeverity::Error);
		return;
	}

	FString PackagePath = BlueprintPackagePathTextBox->GetText().ToString();
	if (PackagePath.IsEmpty())
	{
		PackagePath = TEXT("/Game/Blueprints");
	}

	FString CleanPackagePath = PackagePath;
	CleanPackagePath.RemoveFromEnd(TEXT("/"));
	const FString PackageName = CleanPackagePath + TEXT("/") + ClassDesc.ClassName;

	if (FPackageName::DoesPackageExist(PackageName))
	{
		const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(FString::Printf(TEXT("Blueprint asset '%s' already exists. Overwrite?"), *PackageName)));
		if (Choice != EAppReturnType::Yes)
		{
			SetStatus(TEXT("Create cancelled, existing Blueprint was not overwritten."), EBPStatusSeverity::Info);
			return;
		}
	}

	TArray<FString> Warnings;
	const bool bCreated = FBlueprintAssetWriter::CreateBlueprintAsset(ClassDesc, PackagePath, Warnings);

	if (!bCreated)
	{
		SetStatus(TEXT("Failed to create Blueprint: ") + FString::Join(Warnings, TEXT(" | ")), EBPStatusSeverity::Error);
		return;
	}

	SetStatus(
		Warnings.Num() > 0
			? FString::Printf(TEXT("Created %s/%s with %d warning(s): %s"), *PackagePath, *ClassDesc.ClassName, Warnings.Num(), *FString::Join(Warnings, TEXT(" | ")))
			: FString::Printf(TEXT("Created %s/%s"), *PackagePath, *ClassDesc.ClassName),
		Warnings.Num() > 0 ? EBPStatusSeverity::Warning : EBPStatusSeverity::Success);
}

FReply SBPToCppGenPanel::OnBrowseFolderClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FReply::Handled();
	}

	FString SelectedFolder;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(ParentWindowHandle, TEXT("Select Target Source Folder"), TEXT(""), SelectedFolder);

	if (bFolderSelected)
	{
		OutputFolderTextBox->SetText(FText::FromString(SelectedFolder));
	}

	return FReply::Handled();
}

FReply SBPToCppGenPanel::OnWriteFilesClicked()
{
	if (!bHasGenerated)
	{
		SetStatus(TEXT("Nothing to write yet, click Generate first."), EBPStatusSeverity::Warning);
		return FReply::Handled();
	}

	const FString Folder = OutputFolderTextBox->GetText().ToString();
	if (Folder.IsEmpty())
	{
		SetStatus(TEXT("Choose a target Source folder first."), EBPStatusSeverity::Warning);
		return FReply::Handled();
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Folder))
	{
		PlatformFile.CreateDirectoryTree(*Folder);
	}

	if (OutputTarget == EBPOutputTarget::Cpp)
	{
		const FString HeaderPath = Folder / LastGenerated.HeaderFileName;
		const FString CppPath = Folder / LastGenerated.CppFileName;

		if (FPaths::FileExists(HeaderPath) || FPaths::FileExists(CppPath))
		{
			const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(FString::Printf(TEXT("%s and/or %s already exist in this folder. Overwrite?"), *LastGenerated.HeaderFileName, *LastGenerated.CppFileName)));
			if (Choice != EAppReturnType::Yes)
			{
				SetStatus(TEXT("Write cancelled, existing files were not overwritten."), EBPStatusSeverity::Info);
				return FReply::Handled();
			}
		}

		const bool bHeaderSaved = FFileHelper::SaveStringToFile(LastGenerated.HeaderText, *HeaderPath);
		const bool bCppSaved = FFileHelper::SaveStringToFile(LastGenerated.CppText, *CppPath);

		if (bHeaderSaved && bCppSaved)
		{
			SetStatus(FString::Printf(TEXT("Wrote %s and %s"), *HeaderPath, *CppPath), EBPStatusSeverity::Success);
		}
		else
		{
			SetStatus(TEXT("Failed to write one or both files, check folder permissions."), EBPStatusSeverity::Error);
		}
	}
	else
	{
		const FString VersePath = Folder / LastGeneratedVerse.FileName;

		if (FPaths::FileExists(VersePath))
		{
			const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(FString::Printf(TEXT("%s already exists in this folder. Overwrite?"), *LastGeneratedVerse.FileName)));
			if (Choice != EAppReturnType::Yes)
			{
				SetStatus(TEXT("Write cancelled, existing file was not overwritten."), EBPStatusSeverity::Info);
				return FReply::Handled();
			}
		}

		const bool bVerseSaved = FFileHelper::SaveStringToFile(LastGeneratedVerse.VerseText, *VersePath);

		SetStatus(bVerseSaved ? FString::Printf(TEXT("Wrote %s"), *VersePath) : TEXT("Failed to write the Verse file, check folder permissions."), bVerseSaved ? EBPStatusSeverity::Success : EBPStatusSeverity::Error);
	}

	return FReply::Handled();
}

FText SBPToCppGenPanel::GetHeaderPreviewText() const
{
	if (!bHasGenerated)
	{
		return FText::GetEmpty();
	}
	return (OutputTarget == EBPOutputTarget::Cpp) ? FText::FromString(LastGenerated.HeaderText) : FText::FromString(LastGeneratedVerse.VerseText);
}

FText SBPToCppGenPanel::GetCppPreviewText() const
{
	if (!bHasGenerated || OutputTarget != EBPOutputTarget::Cpp)
	{
		return FText::GetEmpty();
	}
	return FText::FromString(LastGenerated.CppText);
}

FText SBPToCppGenPanel::GetHeaderPaneTitleText() const
{
	return (OutputTarget == EBPOutputTarget::Cpp) ? FText::FromString(TEXT("Header (.h)")) : FText::FromString(TEXT("Verse (.verse)"));
}

FText SBPToCppGenPanel::GetStatusText() const
{
	return FText::FromString(StatusMessage);
}

FSlateColor SBPToCppGenPanel::GetStatusColor() const
{
	switch (StatusSeverity)
	{
	case EBPStatusSeverity::Success:
		return FSlateColor(FLinearColor(0.3f, 0.85f, 0.3f));
	case EBPStatusSeverity::Warning:
		return FSlateColor(FLinearColor(0.95f, 0.65f, 0.15f));
	case EBPStatusSeverity::Error:
		return FSlateColor(FLinearColor(0.9f, 0.25f, 0.25f));
	default:
		return FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f));
	}
}