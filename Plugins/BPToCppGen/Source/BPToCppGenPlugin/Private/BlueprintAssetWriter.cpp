#include "BlueprintAssetWriter.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "GameFramework/Actor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"

FEdGraphPinType FBlueprintAssetWriter::StringTypeToPinType(const FString& CppType, TArray<FString>& OutWarnings)
{
	FEdGraphPinType PinType;
	const FString Type = CppType.TrimStartAndEnd();

	if (Type.StartsWith(TEXT("TArray<")) && Type.EndsWith(TEXT(">")))
	{
		FEdGraphPinType Inner = StringTypeToPinType(Type.Mid(7, Type.Len() - 8), OutWarnings);
		Inner.ContainerType = EPinContainerType::Array;
		return Inner;
	}

	if (Type.StartsWith(TEXT("TSubclassOf<")) && Type.EndsWith(TEXT(">")))
	{
		const FString Inner = Type.Mid(12, Type.Len() - 13);
		PinType.PinCategory = UEdGraphSchema_K2::PC_Class;
		UClass* ResolvedClass = FindFirstObject<UClass>(*Inner);
		if (!ResolvedClass)
		{
			OutWarnings.Add(FString::Printf(TEXT("Could not resolve class '%s' for TSubclassOf, defaulted to UObject"), *Inner));
			ResolvedClass = UObject::StaticClass();
		}
		PinType.PinSubCategoryObject = ResolvedClass;
		return PinType;
	}

	if (Type.EndsWith(TEXT("*")))
	{
		const FString ClassName = Type.LeftChop(1);
		PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		UClass* ResolvedClass = FindFirstObject<UClass>(*ClassName);
		if (!ResolvedClass)
		{
			OutWarnings.Add(FString::Printf(TEXT("Could not resolve class '%s', defaulted to AActor"), *ClassName));
			ResolvedClass = AActor::StaticClass();
		}
		PinType.PinSubCategoryObject = ResolvedClass;
		return PinType;
	}

	if (Type.Equals(TEXT("bool")))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		return PinType;
	}
	if (Type.Equals(TEXT("int32")) || Type.Equals(TEXT("int64")) || Type.Equals(TEXT("uint8")))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
		return PinType;
	}
	if (Type.Equals(TEXT("float")) || Type.Equals(TEXT("double")))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
		PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
		return PinType;
	}
	if (Type.Equals(TEXT("FString")))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_String;
		return PinType;
	}
	if (Type.Equals(TEXT("FName")))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
		return PinType;
	}
	if (Type.Equals(TEXT("FText")))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
		return PinType;
	}

	OutWarnings.Add(FString::Printf(TEXT("Unrecognized type '%s', defaulted to wildcard, fix manually in the Blueprint editor"), *Type));
	PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
	return PinType;
}

bool FBlueprintAssetWriter::CreateBlueprintAsset(const FBPClassDesc& ClassDesc, const FString& PackagePath, TArray<FString>& OutWarnings)
{
	UClass* ParentClass = FindFirstObject<UClass>(*ClassDesc.ParentClass);
	if (!ParentClass)
	{
		OutWarnings.Add(FString::Printf(TEXT("Could not resolve parent class '%s', defaulted to AActor"), *ClassDesc.ParentClass));
		ParentClass = AActor::StaticClass();
	}

	FString CleanPackagePath = PackagePath;
	CleanPackagePath.RemoveFromEnd(TEXT("/"));
	const FString PackageName = CleanPackagePath + TEXT("/") + ClassDesc.ClassName;

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		OutWarnings.Add(TEXT("Failed to create package"));
		return false;
	}

	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		FName(*ClassDesc.ClassName),
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		NAME_None);

	if (!NewBP)
	{
		OutWarnings.Add(TEXT("FKismetEditorUtilities::CreateBlueprint failed"));
		return false;
	}

	for (const FBPVariableDesc& Variable : ClassDesc.Variables)
	{
		const FEdGraphPinType PinType = StringTypeToPinType(Variable.Type, OutWarnings);
		const bool bAdded = FBlueprintEditorUtils::AddMemberVariable(NewBP, FName(*Variable.Name), PinType, Variable.DefaultValue);
		if (!bAdded)
		{
			OutWarnings.Add(FString::Printf(TEXT("Failed to add variable '%s'"), *Variable.Name));
		}
	}

	for (const FBPFunctionDesc& Function : ClassDesc.Functions)
	{
		UEdGraph* FuncGraph = FBlueprintEditorUtils::CreateNewGraph(NewBP, FName(*Function.Name), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
		FBlueprintEditorUtils::AddFunctionGraph<UClass>(NewBP, FuncGraph, true, static_cast<UClass*>(nullptr));

		UK2Node_FunctionEntry* EntryNode = nullptr;
		UK2Node_FunctionResult* ResultNode = nullptr;
		for (UEdGraphNode* Node : FuncGraph->Nodes)
		{
			if (UK2Node_FunctionEntry* AsEntry = Cast<UK2Node_FunctionEntry>(Node))
			{
				EntryNode = AsEntry;
			}
			else if (UK2Node_FunctionResult* AsResult = Cast<UK2Node_FunctionResult>(Node))
			{
				ResultNode = AsResult;
			}
		}

		if (EntryNode)
		{
			for (const FBPParamDesc& Param : Function.Params)
			{
				const FEdGraphPinType ParamPinType = StringTypeToPinType(Param.Type, OutWarnings);
				EntryNode->CreateUserDefinedPin(FName(*Param.Name), ParamPinType, EGPD_Output);
			}
		}
		else
		{
			OutWarnings.Add(FString::Printf(TEXT("No entry node found for function '%s', parameters not added"), *Function.Name));
		}

		if (!Function.ReturnType.Equals(TEXT("void")))
		{
			if (ResultNode)
			{
				const FEdGraphPinType ReturnPinType = StringTypeToPinType(Function.ReturnType, OutWarnings);
				ResultNode->CreateUserDefinedPin(FName(TEXT("ReturnValue")), ReturnPinType, EGPD_Input);
			}
			else
			{
				OutWarnings.Add(FString::Printf(TEXT("No result node found for function '%s', return value not added"), *Function.Name));
			}
		}
	}

	if (ClassDesc.Events.Num() > 0)
	{
		OutWarnings.Add(FString::Printf(TEXT("Skipped %d event(s); overrides and custom events are not created in this MVP, add them manually in the Blueprint editor"), ClassDesc.Events.Num()));
	}

	FKismetEditorUtilities::CompileBlueprint(NewBP);

	FAssetRegistryModule::AssetCreated(NewBP);
	Package->MarkPackageDirty();

	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	const bool bSaved = UPackage::SavePackage(Package, NewBP, *PackageFileName, SaveArgs);

	if (!bSaved)
	{
		OutWarnings.Add(TEXT("Blueprint created in memory but failed to save to disk"));
	}

	return true;
}
