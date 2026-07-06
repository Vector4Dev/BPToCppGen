#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboButton.h"
#include "CppSkeletonGenerator.h"

struct FAssetData;

class SBPToCppGenPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPToCppGenPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnGenerateClicked();
	FReply OnWriteFilesClicked();
	FReply OnBrowseFolderClicked();
	TSharedRef<SWidget> BuildBlueprintPickerMenu();
	void OnBlueprintAssetSelected(const FAssetData& AssetData);

	FText GetHeaderPreviewText() const;
	FText GetCppPreviewText() const;
	FText GetStatusText() const;

	TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
	TSharedPtr<SEditableTextBox> ApiMacroTextBox;
	TSharedPtr<SEditableTextBox> OutputFolderTextBox;
	TSharedPtr<SComboButton> BlueprintPickerButton;

	FGeneratedBPCode LastGenerated;
	bool bHasGenerated = false;
	FString StatusMessage;
};