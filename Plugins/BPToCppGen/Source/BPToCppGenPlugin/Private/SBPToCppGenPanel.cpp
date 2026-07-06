#include "SBPToCppGenPanel.h"
#include "BPDescriptionParser.h"
#include "BlueprintAssetReader.h"
#include "Widgets/Layout/SScrollBox.h"
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

void SBPToCppGenPanel::Construct(const FArguments& InArgs)
{
	StatusMessage = TEXT("Paste a Blueprint description and click Generate.");

	static const FString SampleText =
		TEXT("Class: BP_EnemyController\nParent: AAIController\n\nVariables:\n  Health (float) = 100.0\n  IsAlerted (bool) = false\n  TargetActor (AActor*)\n\nFunctions:\n  TakeDamage(Amount: float) -> void\n  FindNearestTarget() -> AActor*\n\nEvents:\n  BeginPlay (override)\n  Tick (override, DeltaTime: float)\n");

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
					SNew(STextBlock).Text(FText::FromString(TEXT("Blueprint Description")))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(InputTextBox, SMultiLineEditableTextBox)
					.Text(FText::FromString(SampleText))
					.AllowMultiLine(true)
					.AutoWrapText(false)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.Padding(0.f, 0.f, 4.f, 0.f)
					[
						SAssignNew(BlueprintPickerButton, SComboButton)
						.ButtonContent()
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("Load from Blueprint...")))
						]
						.OnGetMenuContent(this, &SBPToCppGenPanel::BuildBlueprintPickerMenu)
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Generate")))
						.HAlign(HAlign_Center)
						.OnClicked(this, &SBPToCppGenPanel::OnGenerateClicked)
					]
				]
			]

			+ SSplitter::Slot()
			.Value(0.3f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Header (.h)")))
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
			.Value(0.3f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Source (.cpp)")))
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

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.f, 4.f)
		[
			SNew(SHorizontalBox)
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
		.Padding(8.f, 0.f, 8.f, 8.f)
		[
			SNew(STextBlock).Text(this, &SBPToCppGenPanel::GetStatusText)
		]
	];
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
		StatusMessage = TEXT("Selected asset is not a Blueprint.");
		return;
	}

	FBPClassDesc ClassDesc;
	TArray<FString> Warnings;
	const bool bRead = FBlueprintAssetReader::ReadBlueprint(Blueprint, ClassDesc, Warnings);

	if (!bRead)
	{
		StatusMessage = TEXT("Failed to read Blueprint: ") + FString::Join(Warnings, TEXT(" | "));
		return;
	}

	InputTextBox->SetText(FText::FromString(FBPDescriptionParser::Serialize(ClassDesc)));

	if (Warnings.Num() > 0)
	{
		StatusMessage = FString::Printf(TEXT("Loaded %s with %d warning(s): %s"), *ClassDesc.ClassName, Warnings.Num(), *FString::Join(Warnings, TEXT(" | ")));
	}
	else
	{
		StatusMessage = FString::Printf(TEXT("Loaded %s, review then click Generate."), *ClassDesc.ClassName);
	}
}

FReply SBPToCppGenPanel::OnGenerateClicked()
{
	const FString SourceText = InputTextBox->GetText().ToString();
	const FString ApiMacro = ApiMacroTextBox->GetText().ToString();

	FBPClassDesc ClassDesc;
	TArray<FString> Errors;
	const bool bParsed = FBPDescriptionParser::Parse(SourceText, ClassDesc, Errors);

	if (!bParsed)
	{
		bHasGenerated = false;
		StatusMessage = TEXT("Failed to parse: ") + FString::Join(Errors, TEXT(" | "));
		return FReply::Handled();
	}

	LastGenerated = FCppSkeletonGenerator::Generate(ClassDesc, ApiMacro);
	bHasGenerated = true;

	if (Errors.Num() > 0)
	{
		StatusMessage = FString::Printf(TEXT("Generated with %d warning(s): %s"), Errors.Num(), *FString::Join(Errors, TEXT(" | ")));
	}
	else
	{
		StatusMessage = FString::Printf(TEXT("Generated %s / %s"), *LastGenerated.HeaderFileName, *LastGenerated.CppFileName);
	}

	return FReply::Handled();
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
		StatusMessage = TEXT("Nothing to write yet, click Generate first.");
		return FReply::Handled();
	}

	const FString Folder = OutputFolderTextBox->GetText().ToString();
	if (Folder.IsEmpty())
	{
		StatusMessage = TEXT("Choose a target Source folder first.");
		return FReply::Handled();
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Folder))
	{
		PlatformFile.CreateDirectoryTree(*Folder);
	}

	const FString HeaderPath = Folder / LastGenerated.HeaderFileName;
	const FString CppPath = Folder / LastGenerated.CppFileName;

	const bool bHeaderSaved = FFileHelper::SaveStringToFile(LastGenerated.HeaderText, *HeaderPath);
	const bool bCppSaved = FFileHelper::SaveStringToFile(LastGenerated.CppText, *CppPath);

	if (bHeaderSaved && bCppSaved)
	{
		StatusMessage = FString::Printf(TEXT("Wrote %s and %s"), *HeaderPath, *CppPath);
	}
	else
	{
		StatusMessage = TEXT("Failed to write one or both files, check folder permissions.");
	}

	return FReply::Handled();
}

FText SBPToCppGenPanel::GetHeaderPreviewText() const
{
	return bHasGenerated ? FText::FromString(LastGenerated.HeaderText) : FText::GetEmpty();
}

FText SBPToCppGenPanel::GetCppPreviewText() const
{
	return bHasGenerated ? FText::FromString(LastGenerated.CppText) : FText::GetEmpty();
}

FText SBPToCppGenPanel::GetStatusText() const
{
	return FText::FromString(StatusMessage);
}