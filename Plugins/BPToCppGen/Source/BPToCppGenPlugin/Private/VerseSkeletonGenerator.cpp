#include "VerseSkeletonGenerator.h"

FString FVerseSkeletonGenerator::ToSnakeCase(const FString& PascalName)
{
	FString Name = PascalName;
	if (Name.StartsWith(TEXT("BP_")))
	{
		Name = Name.RightChop(3);
	}

	FString Result;
	for (int32 Index = 0; Index < Name.Len(); ++Index)
	{
		const TCHAR Current = Name[Index];

		if (Index > 0 && FChar::IsUpper(Current))
		{
			const bool bPrevIsLower = !FChar::IsUpper(Name[Index - 1]);
			const bool bAcronymBoundary = FChar::IsUpper(Name[Index - 1]) && (Index + 1 < Name.Len()) && !FChar::IsUpper(Name[Index + 1]);

			if (bPrevIsLower || bAcronymBoundary)
			{
				Result += TEXT("_");
			}
		}

		Result += FChar::ToLower(Current);
	}

	return Result;
}

FString FVerseSkeletonGenerator::StripPrefixAndLowerFirst(const FString& EngineClassName)
{
	FString Bare = EngineClassName;

	if (Bare.Len() > 1)
	{
		const TCHAR FirstChar = Bare[0];
		if ((FirstChar == 'A' || FirstChar == 'U' || FirstChar == 'S' || FirstChar == 'F' || FirstChar == 'I') && FChar::IsUpper(Bare[1]))
		{
			Bare = Bare.RightChop(1);
		}
	}

	return ToSnakeCase(Bare);
}

FString FVerseSkeletonGenerator::MapTypeToVerse(const FString& CppStyleType)
{
	FString Type = CppStyleType.TrimStartAndEnd();

	if (Type.StartsWith(TEXT("TArray<")) && Type.EndsWith(TEXT(">")))
	{
		const FString Inner = Type.Mid(7, Type.Len() - 8);
		return FString::Printf(TEXT("[]%s"), *MapTypeToVerse(Inner));
	}
	if (Type.StartsWith(TEXT("TSet<")) && Type.EndsWith(TEXT(">")))
	{
		const FString Inner = Type.Mid(5, Type.Len() - 6);
		return FString::Printf(TEXT("[]%s"), *MapTypeToVerse(Inner));
	}
	if (Type.StartsWith(TEXT("TMap<")) && Type.EndsWith(TEXT(">")))
	{
		const FString Inner = Type.Mid(5, Type.Len() - 6);
		FString KeyPart;
		FString ValuePart;
		if (Inner.Split(TEXT(","), &KeyPart, &ValuePart))
		{
			return FString::Printf(TEXT("[%s]%s"), *MapTypeToVerse(KeyPart.TrimStartAndEnd()), *MapTypeToVerse(ValuePart.TrimStartAndEnd()));
		}
		return TEXT("[]string");
	}
	if (Type.StartsWith(TEXT("TSubclassOf<")) && Type.EndsWith(TEXT(">")))
	{
		const FString Inner = Type.Mid(12, Type.Len() - 13);
		return FString::Printf(TEXT("subclass(%s)"), *MapTypeToVerse(Inner));
	}

	if (Type.EndsWith(TEXT("*")))
	{
		const FString Bare = StripPrefixAndLowerFirst(Type.LeftChop(1));
		return FString::Printf(TEXT("?%s"), *Bare);
	}

	static const TMap<FString, FString> DirectMap =
	{
		{ TEXT("float"), TEXT("float") },
		{ TEXT("double"), TEXT("float") },
		{ TEXT("int32"), TEXT("int") },
		{ TEXT("int64"), TEXT("int") },
		{ TEXT("uint8"), TEXT("int") },
		{ TEXT("bool"), TEXT("logic") },
		{ TEXT("FString"), TEXT("string") },
		{ TEXT("FName"), TEXT("string") },
		{ TEXT("FText"), TEXT("string") },
		{ TEXT("void"), TEXT("void") }
	};

	if (const FString* Direct = DirectMap.Find(Type))
	{
		return *Direct;
	}

	if (Type.StartsWith(TEXT("E")) && Type.Len() > 1 && FChar::IsUpper(Type[1]))
	{
		return Type;
	}
	if (Type.StartsWith(TEXT("F")) && Type.Len() > 1 && FChar::IsUpper(Type[1]))
	{
		return Type.RightChop(1);
	}

	return Type;
}

FString FVerseSkeletonGenerator::DefaultVerseValue(const FString& VerseType)
{
	if (VerseType.Equals(TEXT("logic")))
	{
		return TEXT("false");
	}
	if (VerseType.Equals(TEXT("int")))
	{
		return TEXT("0");
	}
	if (VerseType.Equals(TEXT("float")))
	{
		return TEXT("0.0");
	}
	if (VerseType.Equals(TEXT("string")))
	{
		return TEXT("\"\"");
	}
	if (VerseType.Equals(TEXT("void")))
	{
		return TEXT("void");
	}
	if (VerseType.StartsWith(TEXT("?")))
	{
		return TEXT("false");
	}
	if (VerseType.StartsWith(TEXT("[]")))
	{
		return TEXT("array{}");
	}
	if (VerseType.StartsWith(TEXT("[")))
	{
		return TEXT("map{}");
	}
	if (VerseType.StartsWith(TEXT("subclass(")))
	{
		return TEXT("false");
	}
	return TEXT("false");
}

FString FVerseSkeletonGenerator::BuildParamString(const TArray<FBPParamDesc>& Params)
{
	FString Result;
	for (int32 Index = 0; Index < Params.Num(); ++Index)
	{
		Result += FString::Printf(TEXT("%s : %s"), *Params[Index].Name, *MapTypeToVerse(Params[Index].Type));
		if (Index != Params.Num() - 1)
		{
			Result += TEXT(", ");
		}
	}
	return Result;
}

FGeneratedVerseCode FVerseSkeletonGenerator::Generate(const FBPClassDesc& ClassDesc, TArray<FString>& OutWarnings)
{
	FGeneratedVerseCode Result;

	const FString ClassIdentifier = ToSnakeCase(ClassDesc.ClassName);

	if (!ClassDesc.ParentClass.IsEmpty())
	{
		OutWarnings.Add(FString::Printf(TEXT("Parent class '%s' has no UEFN equivalent, generated as a creative_device instead"), *ClassDesc.ParentClass));
	}

	bool bHasPointerType = false;
	for (const FBPVariableDesc& Variable : ClassDesc.Variables)
	{
		bHasPointerType |= Variable.Type.EndsWith(TEXT("*"));
	}
	for (const FBPFunctionDesc& Function : ClassDesc.Functions)
	{
		bHasPointerType |= Function.ReturnType.EndsWith(TEXT("*"));
		for (const FBPParamDesc& Param : Function.Params)
		{
			bHasPointerType |= Param.Type.EndsWith(TEXT("*"));
		}
	}
	if (bHasPointerType)
	{
		OutWarnings.Add(TEXT("Object reference types (AActor*, UObject*, etc.) were mapped to a generic '?type' placeholder; UEFN uses specific types like agent/player instead, review these manually"));
	}

	FString Text;
	Text += TEXT("using { /Fortnite.com/Devices }\n");
	Text += TEXT("using { /Verse.org/Simulation }\n\n");
	Text += FString::Printf(TEXT("%s := class(creative_device):\n"), *ClassIdentifier);

	if (ClassDesc.Variables.Num() > 0)
	{
		Text += TEXT("\n");
		for (const FBPVariableDesc& Variable : ClassDesc.Variables)
		{
			const FString VerseType = MapTypeToVerse(Variable.Type);
			const FString DefaultVal = Variable.bHasDefault ? Variable.DefaultValue : DefaultVerseValue(VerseType);
			Text += TEXT("    @editable\n");
			Text += FString::Printf(TEXT("    var %s : %s = %s\n\n"), *Variable.Name, *VerseType, *DefaultVal);
		}
	}

	TArray<FBPEventDesc> CustomEvents;
	bool bHasBeginPlay = false;
	bool bHasTick = false;

	for (const FBPEventDesc& Event : ClassDesc.Events)
	{
		if (!Event.bOverride)
		{
			CustomEvents.Add(Event);
			continue;
		}

		if (Event.Name.Equals(TEXT("BeginPlay"), ESearchCase::IgnoreCase))
		{
			bHasBeginPlay = true;
		}
		else if (Event.Name.Equals(TEXT("Tick"), ESearchCase::IgnoreCase))
		{
			bHasTick = true;
		}
		else
		{
			OutWarnings.Add(FString::Printf(TEXT("Skipped override event '%s', no UEFN equivalent"), *Event.Name));
		}
	}

	if (bHasBeginPlay || bHasTick)
	{
		Text += TEXT("    OnBegin<override>()<suspends>:void=\n");
		if (bHasTick)
		{
			Text += TEXT("        loop:\n");
			Text += TEXT("            Sleep(0.0)\n");
		}
		else
		{
			Text += TEXT("        void\n");
		}
		Text += TEXT("\n");
	}

	if (ClassDesc.Functions.Num() > 0)
	{
		for (const FBPFunctionDesc& Function : ClassDesc.Functions)
		{
			const FString ReturnType = MapTypeToVerse(Function.ReturnType);
			Text += FString::Printf(TEXT("    %s(%s) : %s =\n"), *Function.Name, *BuildParamString(Function.Params), *ReturnType);
			Text += FString::Printf(TEXT("        %s\n\n"), *DefaultVerseValue(ReturnType));
		}
	}

	if (CustomEvents.Num() > 0)
	{
		for (const FBPEventDesc& Event : CustomEvents)
		{
			Text += FString::Printf(TEXT("    %s(%s) : void =\n"), *Event.Name, *BuildParamString(Event.Params));
			Text += TEXT("        void\n\n");
		}
	}

	Result.FileName = ClassIdentifier + TEXT(".verse");
	Result.VerseText = Text;

	return Result;
}