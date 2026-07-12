#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "CppSkeletonGenerator.h"
#include "VerseSkeletonGenerator.h"
#include "VerseSourceReader.h"
#include "BlueprintAssetWriter.h"

struct FAssetData;

enum class EBPOutputTarget : uint8
{
	Cpp,
	Verse,
	BlueprintAsset
};

enum class EBPInputSource : uint8
{
	PasteText,
	BlueprintAsset,
	VerseFile
};

enum class EBPStatusSeverity : uint8
{
	Info,
	Success,
	Warning,
	Error
};

class SBPToCppGenPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPToCppGenPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnPrimaryActionClicked();
	void RunGenerate();
	void RunCreateBlueprint();

	FReply OnWriteFilesClicked();
	FReply OnBrowseFolderClicked();
	FReply OnImportVerseClicked();
	FReply OnClearClicked();
	TSharedRef<SWidget> BuildBlueprintPickerMenu();
	void OnBlueprintAssetSelected(const FAssetData& AssetData);

	ECheckBoxState IsCppTargetChecked() const;
	ECheckBoxState IsVerseTargetChecked() const;
	ECheckBoxState IsBlueprintTargetChecked() const;
	void OnCppTargetChanged(ECheckBoxState NewState);
	void OnVerseTargetChanged(ECheckBoxState NewState);
	void OnBlueprintTargetChanged(ECheckBoxState NewState);
	void SetOutputTarget(EBPOutputTarget NewTarget);

	ECheckBoxState IsSourcePasteChecked() const;
	ECheckBoxState IsSourceBlueprintChecked() const;
	ECheckBoxState IsSourceVerseChecked() const;
	void OnSourcePasteChanged(ECheckBoxState NewState);
	void OnSourceBlueprintChanged(ECheckBoxState NewState);
	void OnSourceVerseChanged(ECheckBoxState NewState);
	void SetInputSource(EBPInputSource NewSource);

	void SetStatus(const FString& Message, EBPStatusSeverity Severity);

	FText GetHeaderPreviewText() const;
	FText GetCppPreviewText() const;
	FText GetHeaderPaneTitleText() const;
	FText GetStatusText() const;
	FText GetPrimaryActionLabelText() const;
	FSlateColor GetStatusColor() const;
	EVisibility GetApiMacroRowVisibility() const;
	EVisibility GetFolderExportRowVisibility() const;
	EVisibility GetBlueprintExportRowVisibility() const;
	EVisibility GetBlueprintPickerVisibility() const;
	EVisibility GetVerseImportVisibility() const;

	TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
	TSharedPtr<SEditableTextBox> ApiMacroTextBox;
	TSharedPtr<SEditableTextBox> OutputFolderTextBox;
	TSharedPtr<SEditableTextBox> BlueprintPackagePathTextBox;
	TSharedPtr<SComboButton> BlueprintPickerButton;
	TSharedPtr<SWidgetSwitcher> PreviewSwitcher;

	EBPOutputTarget OutputTarget = EBPOutputTarget::Cpp;
	EBPInputSource InputSource = EBPInputSource::PasteText;
	FGeneratedBPCode LastGenerated;
	FGeneratedVerseCode LastGeneratedVerse;
	bool bHasGenerated = false;
	FString StatusMessage;
	EBPStatusSeverity StatusSeverity = EBPStatusSeverity::Info;
};