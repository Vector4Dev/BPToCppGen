#include "BlueprintAssetReader.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_Event.h"

FString FBlueprintAssetReader::ClassToCppName(const UClass* Class)
{
	if (!Class)
	{
		return TEXT("UObject*");
	}
	return FString::Printf(TEXT("%s%s*"), Class->GetPrefixCPP(), *Class->GetName());
}

FString FBlueprintAssetReader::PinTypeToString(const FEdGraphPinType& PinType)
{
	FString BaseType;
	const FName Category = PinType.PinCategory;

	if (Category == UEdGraphSchema_K2::PC_Boolean)
	{
		BaseType = TEXT("bool");
	}
	else if (Category == UEdGraphSchema_K2::PC_Byte)
	{
		const UEnum* Enum = Cast<UEnum>(PinType.PinSubCategoryObject.Get());
		if (Enum && !Enum->CppType.IsEmpty())
		{
			BaseType = Enum->CppType;
		}
		else if (Enum)
		{
			BaseType = TEXT("E") + Enum->GetName();
		}
		else
		{
			BaseType = TEXT("uint8");
		}
	}
	else if (Category == UEdGraphSchema_K2::PC_Int)
	{
		BaseType = TEXT("int32");
	}
	else if (Category == UEdGraphSchema_K2::PC_Int64)
	{
		BaseType = TEXT("int64");
	}
	else if (Category == UEdGraphSchema_K2::PC_Real)
	{
		BaseType = (PinType.PinSubCategory == UEdGraphSchema_K2::PC_Float) ? TEXT("float") : TEXT("double");
	}
	else if (Category == UEdGraphSchema_K2::PC_Float)
	{
		BaseType = TEXT("float");
	}
	else if (Category == UEdGraphSchema_K2::PC_Name)
	{
		BaseType = TEXT("FName");
	}
	else if (Category == UEdGraphSchema_K2::PC_String)
	{
		BaseType = TEXT("FString");
	}
	else if (Category == UEdGraphSchema_K2::PC_Text)
	{
		BaseType = TEXT("FText");
	}
	else if (Category == UEdGraphSchema_K2::PC_Struct)
	{
		const UScriptStruct* Struct = Cast<UScriptStruct>(PinType.PinSubCategoryObject.Get());
		BaseType = Struct ? (TEXT("F") + Struct->GetName()) : TEXT("FVector");
	}
	else if (Category == UEdGraphSchema_K2::PC_Object
		|| Category == UEdGraphSchema_K2::PC_Interface
		|| Category == UEdGraphSchema_K2::PC_SoftObject)
	{
		BaseType = ClassToCppName(Cast<UClass>(PinType.PinSubCategoryObject.Get()));
	}
	else if (Category == UEdGraphSchema_K2::PC_Class || Category == UEdGraphSchema_K2::PC_SoftClass)
	{
		const UClass* Class = Cast<UClass>(PinType.PinSubCategoryObject.Get());
		BaseType = Class ? FString::Printf(TEXT("TSubclassOf<%s%s>"), Class->GetPrefixCPP(), *Class->GetName()) : TEXT("UClass*");
	}
	else
	{
		BaseType = TEXT("FString");
	}

	if (PinType.ContainerType == EPinContainerType::Array)
	{
		return FString::Printf(TEXT("TArray<%s>"), *BaseType);
	}
	if (PinType.ContainerType == EPinContainerType::Set)
	{
		return FString::Printf(TEXT("TSet<%s>"), *BaseType);
	}
	if (PinType.ContainerType == EPinContainerType::Map)
	{
		FString ValueType = TEXT("FString");
		if (PinType.PinValueType.TerminalCategory == UEdGraphSchema_K2::PC_Boolean)
		{
			ValueType = TEXT("bool");
		}
		else if (PinType.PinValueType.TerminalCategory == UEdGraphSchema_K2::PC_Int)
		{
			ValueType = TEXT("int32");
		}
		else if (PinType.PinValueType.TerminalCategory == UEdGraphSchema_K2::PC_Real)
		{
			ValueType = TEXT("float");
		}
		return FString::Printf(TEXT("TMap<%s, %s>"), *BaseType, *ValueType);
	}

	return BaseType;
}

FString FBlueprintAssetReader::MakeSafeIdentifier(const FString& RawName)
{
	FString Result;
	for (const TCHAR Ch : RawName)
	{
		if (FChar::IsAlnum(Ch) || Ch == TEXT('_'))
		{
			Result += Ch;
		}
	}

	if (Result.IsEmpty())
	{
		return TEXT("UnnamedIdentifier");
	}

	if (FChar::IsDigit(Result[0]))
	{
		Result = TEXT("_") + Result;
	}

	return Result;
}

bool FBlueprintAssetReader::ReadBlueprint(UBlueprint* Blueprint, FBPClassDesc& OutClassDesc, TArray<FString>& OutWarnings)
{
	if (!Blueprint)
	{
		OutWarnings.Add(TEXT("No Blueprint asset provided"));
		return false;
	}

	OutClassDesc.ClassName = Blueprint->GetName();
	OutClassDesc.ParentClass = ClassToCppName(Blueprint->ParentClass).LeftChop(1);

	for (const FBPVariableDescription& VarDesc : Blueprint->NewVariables)
	{
		FBPVariableDesc Variable;
		Variable.Name = MakeSafeIdentifier(VarDesc.VarName.ToString());
		if (Variable.Name != VarDesc.VarName.ToString())
		{
			OutWarnings.Add(FString::Printf(TEXT("Variable name '%s' contained invalid characters, sanitized to '%s'"), *VarDesc.VarName.ToString(), *Variable.Name));
		}
		Variable.Type = PinTypeToString(VarDesc.VarType);
		if (!VarDesc.DefaultValue.IsEmpty())
		{
			Variable.DefaultValue = VarDesc.DefaultValue;
			Variable.bHasDefault = true;
		}
		OutClassDesc.Variables.Add(Variable);
	}

	for (UEdGraph* Graph : Blueprint->FunctionGraphs)
	{
		if (!Graph)
		{
			continue;
		}

		UK2Node_FunctionEntry* EntryNode = nullptr;
		UK2Node_FunctionResult* ResultNode = nullptr;

		for (UEdGraphNode* Node : Graph->Nodes)
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

		if (!EntryNode)
		{
			OutWarnings.Add(FString::Printf(TEXT("Skipped graph with no entry node: %s"), *Graph->GetName()));
			continue;
		}

		FBPFunctionDesc Function;
		const FString RawFunctionName = Graph->GetName();
		Function.Name = MakeSafeIdentifier(RawFunctionName);
		if (Function.Name != RawFunctionName)
		{
			OutWarnings.Add(FString::Printf(TEXT("Function name '%s' contained invalid characters, sanitized to '%s'"), *RawFunctionName, *Function.Name));
		}

		for (const TSharedPtr<FUserPinInfo>& PinInfo : EntryNode->UserDefinedPins)
		{
			if (!PinInfo.IsValid())
			{
				continue;
			}
			FBPParamDesc Param;
			Param.Name = MakeSafeIdentifier(PinInfo->PinName.ToString());
			Param.Type = PinTypeToString(PinInfo->PinType);
			Function.Params.Add(Param);
		}

		Function.ReturnType = TEXT("void");
		if (ResultNode && ResultNode->UserDefinedPins.Num() > 0 && ResultNode->UserDefinedPins[0].IsValid())
		{
			Function.ReturnType = PinTypeToString(ResultNode->UserDefinedPins[0]->PinType);
		}

		OutClassDesc.Functions.Add(Function);
	}

	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		if (!Graph)
		{
			continue;
		}

		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (UK2Node_CustomEvent* CustomEvent = Cast<UK2Node_CustomEvent>(Node))
			{
				FBPEventDesc Event;
				const FString RawEventName = CustomEvent->CustomFunctionName.ToString();
				Event.Name = MakeSafeIdentifier(RawEventName);
				if (Event.Name != RawEventName)
				{
					OutWarnings.Add(FString::Printf(TEXT("Event name '%s' contained invalid characters, sanitized to '%s'"), *RawEventName, *Event.Name));
				}
				Event.bOverride = false;

				for (const TSharedPtr<FUserPinInfo>& PinInfo : CustomEvent->UserDefinedPins)
				{
					if (!PinInfo.IsValid())
					{
						continue;
					}
					FBPParamDesc Param;
					Param.Name = MakeSafeIdentifier(PinInfo->PinName.ToString());
					Param.Type = PinTypeToString(PinInfo->PinType);
					Event.Params.Add(Param);
				}

				OutClassDesc.Events.Add(Event);
			}
			else if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
			{
				if (!EventNode->bOverrideFunction)
				{
					continue;
				}

				FBPEventDesc Event;
				const FString RawEventName = EventNode->EventReference.GetMemberName().ToString();
				Event.Name = MakeSafeIdentifier(RawEventName);
				if (Event.Name != RawEventName)
				{
					OutWarnings.Add(FString::Printf(TEXT("Event name '%s' contained invalid characters (likely an Enhanced Input action display name), sanitized to '%s'"), *RawEventName, *Event.Name));
				}
				Event.bOverride = true;

				for (UEdGraphPin* Pin : EventNode->Pins)
				{
					if (!Pin || Pin->Direction != EGPD_Output)
					{
						continue;
					}
					if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
					{
						continue;
					}
					FBPParamDesc Param;
					Param.Name = MakeSafeIdentifier(Pin->PinName.ToString());
					Param.Type = PinTypeToString(Pin->PinType);
					Event.Params.Add(Param);
				}

				OutClassDesc.Events.Add(Event);
			}
		}
	}

	if (Blueprint->MacroGraphs.Num() > 0)
	{
		OutWarnings.Add(FString::Printf(TEXT("Skipped %d macro graph(s), not supported"), Blueprint->MacroGraphs.Num()));
	}
	if (Blueprint->ImplementedInterfaces.Num() > 0)
	{
		OutWarnings.Add(FString::Printf(TEXT("Skipped %d implemented interface(s), not supported"), Blueprint->ImplementedInterfaces.Num()));
	}

	return true;
}