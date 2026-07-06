#include "CppSkeletonGenerator.h"

FString FCppSkeletonGenerator::MapType(const FString& BPType)
{
	static const TMap<FString, FString> TypeMap =
	{
		{ TEXT("Integer"), TEXT("int32") },
		{ TEXT("Int"), TEXT("int32") },
		{ TEXT("Float"), TEXT("float") },
		{ TEXT("Boolean"), TEXT("bool") },
		{ TEXT("Bool"), TEXT("bool") },
		{ TEXT("String"), TEXT("FString") },
		{ TEXT("Text"), TEXT("FText") },
		{ TEXT("Name"), TEXT("FName") },
		{ TEXT("Byte"), TEXT("uint8") },
		{ TEXT("Vector"), TEXT("FVector") },
		{ TEXT("Vector2D"), TEXT("FVector2D") },
		{ TEXT("Rotator"), TEXT("FRotator") },
		{ TEXT("Transform"), TEXT("FTransform") },
		{ TEXT("Object"), TEXT("UObject*") },
		{ TEXT("Class"), TEXT("UClass*") },
		{ TEXT("Actor"), TEXT("AActor*") }
	};

	const FString* Mapped = TypeMap.Find(BPType);
	return Mapped ? *Mapped : BPType;
}

FString FCppSkeletonGenerator::InferClassPrefix(const FString& ParentClass)
{
	if (ParentClass.Len() > 0)
	{
		const TCHAR FirstChar = ParentClass[0];
		if (FirstChar == 'A' || FirstChar == 'U' || FirstChar == 'S' || FirstChar == 'F' || FirstChar == 'I')
		{
			return FString(1, &FirstChar);
		}
	}
	return TEXT("A");
}

FString FCppSkeletonGenerator::ApplyClassPrefix(const FString& ClassName, const FString& Prefix)
{
	if (ClassName.Len() > 0)
	{
		const TCHAR FirstChar = ClassName[0];
		if (FirstChar == 'A' || FirstChar == 'U' || FirstChar == 'S' || FirstChar == 'F' || FirstChar == 'I')
		{
			return ClassName;
		}
	}
	return Prefix + ClassName;
}

FString FCppSkeletonGenerator::GuessParentHeader(const FString& ParentClass)
{
	if (ParentClass.Len() > 1)
	{
		return ParentClass.RightChop(1) + TEXT(".h");
	}
	return ParentClass + TEXT(".h");
}

FString FCppSkeletonGenerator::BuildParamString(const TArray<FBPParamDesc>& Params)
{
	FString Result;
	for (int32 Index = 0; Index < Params.Num(); ++Index)
	{
		Result += MapType(Params[Index].Type) + TEXT(" ") + Params[Index].Name;
		if (Index != Params.Num() - 1)
		{
			Result += TEXT(", ");
		}
	}
	return Result;
}

FString FCppSkeletonGenerator::BuildParamNameList(const TArray<FBPParamDesc>& Params)
{
	FString Result;
	for (int32 Index = 0; Index < Params.Num(); ++Index)
	{
		Result += Params[Index].Name;
		if (Index != Params.Num() - 1)
		{
			Result += TEXT(", ");
		}
	}
	return Result;
}

FString FCppSkeletonGenerator::DefaultReturnValue(const FString& ReturnType)
{
	if (ReturnType.Equals(TEXT("void")))
	{
		return FString();
	}
	if (ReturnType.EndsWith(TEXT("*")))
	{
		return TEXT("nullptr");
	}
	if (ReturnType.Equals(TEXT("bool")))
	{
		return TEXT("false");
	}
	if (ReturnType.Equals(TEXT("int32")) || ReturnType.Equals(TEXT("float")) || ReturnType.Equals(TEXT("double")) || ReturnType.Equals(TEXT("uint8")))
	{
		return TEXT("0");
	}
	return TEXT("{}");
}

bool FCppSkeletonGenerator::FindKnownOverride(const FString& EventName, FKnownOverrideInfo& OutInfo)
{
	static const TMap<FString, FKnownOverrideInfo> KnownOverrides =
	{
		{ TEXT("BeginPlay"), { TEXT("void"), TEXT(""), TEXT("") } },
		{ TEXT("EndPlay"), { TEXT("void"), TEXT("const EEndPlayReason::Type EndPlayReason"), TEXT("EndPlayReason") } },
		{ TEXT("Tick"), { TEXT("void"), TEXT("float DeltaTime"), TEXT("DeltaTime") } },
		{ TEXT("OnConstruction"), { TEXT("void"), TEXT("const FTransform& Transform"), TEXT("Transform") } },
		{ TEXT("NativeConstruct"), { TEXT("void"), TEXT(""), TEXT("") } },
		{ TEXT("NativeDestruct"), { TEXT("void"), TEXT(""), TEXT("") } },
		{ TEXT("Destroyed"), { TEXT("void"), TEXT(""), TEXT("") } },
		{ TEXT("PostInitializeComponents"), { TEXT("void"), TEXT(""), TEXT("") } }
	};

	const FKnownOverrideInfo* Found = KnownOverrides.Find(EventName);
	if (Found)
	{
		OutInfo = *Found;
		return true;
	}
	return false;
}

FGeneratedBPCode FCppSkeletonGenerator::Generate(const FBPClassDesc& ClassDesc, const FString& ApiMacro)
{
	FGeneratedBPCode Result;

	const FString Prefix = InferClassPrefix(ClassDesc.ParentClass);
	const FString FullClassName = ApplyClassPrefix(ClassDesc.ClassName, Prefix);
	const FString BareClassName = FullClassName.RightChop(1);

	Result.HeaderFileName = BareClassName + TEXT(".h");
	Result.CppFileName = BareClassName + TEXT(".cpp");

	FString Header;
	Header += TEXT("#pragma once\n\n");
	Header += TEXT("#include \"CoreMinimal.h\"\n");
	Header += FString::Printf(TEXT("#include \"%s\"\n"), *GuessParentHeader(ClassDesc.ParentClass));
	Header += FString::Printf(TEXT("#include \"%s.generated.h\"\n\n"), *BareClassName);
	Header += TEXT("UCLASS()\n");
	Header += FString::Printf(TEXT("class %s %s : public %s\n"), *ApiMacro, *FullClassName, *ClassDesc.ParentClass);
	Header += TEXT("{\n");
	Header += TEXT("\tGENERATED_BODY()\n\n");
	Header += TEXT("public:\n");
	Header += FString::Printf(TEXT("\t%s();\n"), *FullClassName);

	if (ClassDesc.Variables.Num() > 0)
	{
		Header += TEXT("\n");
		for (const FBPVariableDesc& Variable : ClassDesc.Variables)
		{
			Header += TEXT("\tUPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Variables\")\n");
			if (Variable.bHasDefault)
			{
				Header += FString::Printf(TEXT("\t%s %s = %s;\n"), *MapType(Variable.Type), *Variable.Name, *Variable.DefaultValue);
			}
			else
			{
				Header += FString::Printf(TEXT("\t%s %s;\n"), *MapType(Variable.Type), *Variable.Name);
			}
		}
	}

	if (ClassDesc.Functions.Num() > 0)
	{
		Header += TEXT("\n");
		for (const FBPFunctionDesc& Function : ClassDesc.Functions)
		{
			Header += TEXT("\tUFUNCTION(BlueprintCallable, Category = \"Functions\")\n");
			Header += FString::Printf(TEXT("\t%s %s(%s);\n"), *MapType(Function.ReturnType), *Function.Name, *BuildParamString(Function.Params));
		}
	}

	TArray<FBPEventDesc> NativeEvents;
	TArray<FBPEventDesc> OverrideEvents;
	for (const FBPEventDesc& Event : ClassDesc.Events)
	{
		if (Event.bOverride)
		{
			OverrideEvents.Add(Event);
		}
		else
		{
			NativeEvents.Add(Event);
		}
	}

	if (NativeEvents.Num() > 0)
	{
		Header += TEXT("\n");
		for (const FBPEventDesc& Event : NativeEvents)
		{
			const FString ParamString = BuildParamString(Event.Params);
			Header += TEXT("\tUFUNCTION(BlueprintNativeEvent, Category = \"Events\")\n");
			Header += FString::Printf(TEXT("\tvoid %s(%s);\n"), *Event.Name, *ParamString);
			Header += FString::Printf(TEXT("\tvirtual void %s_Implementation(%s);\n"), *Event.Name, *ParamString);
		}
	}

	if (OverrideEvents.Num() > 0)
	{
		Header += TEXT("\nprotected:\n");
		for (const FBPEventDesc& Event : OverrideEvents)
		{
			FKnownOverrideInfo KnownInfo;
			if (FindKnownOverride(Event.Name, KnownInfo))
			{
				Header += FString::Printf(TEXT("\tvirtual %s %s(%s) override;\n"), *KnownInfo.ReturnType, *Event.Name, *KnownInfo.Params);
			}
			else
			{
				Header += FString::Printf(TEXT("\tvirtual void %s(%s) override;\n"), *Event.Name, *BuildParamString(Event.Params));
			}
		}
	}

	Header += TEXT("};\n");
	Result.HeaderText = Header;

	FString Cpp;
	Cpp += FString::Printf(TEXT("#include \"%s\"\n\n"), *Result.HeaderFileName);
	Cpp += FString::Printf(TEXT("%s::%s()\n{\n}\n"), *FullClassName, *FullClassName);

	for (const FBPFunctionDesc& Function : ClassDesc.Functions)
	{
		const FString MappedReturn = MapType(Function.ReturnType);
		Cpp += TEXT("\n");
		Cpp += FString::Printf(TEXT("%s %s::%s(%s)\n{\n"), *MappedReturn, *FullClassName, *Function.Name, *BuildParamString(Function.Params));
		if (!MappedReturn.Equals(TEXT("void")))
		{
			Cpp += FString::Printf(TEXT("\treturn %s;\n"), *DefaultReturnValue(MappedReturn));
		}
		Cpp += TEXT("}\n");
	}

	for (const FBPEventDesc& Event : NativeEvents)
	{
		Cpp += TEXT("\n");
		Cpp += FString::Printf(TEXT("void %s::%s_Implementation(%s)\n{\n}\n"), *FullClassName, *Event.Name, *BuildParamString(Event.Params));
	}

	for (const FBPEventDesc& Event : OverrideEvents)
	{
		FKnownOverrideInfo KnownInfo;
		FString ReturnType = TEXT("void");
		FString ParamString = BuildParamString(Event.Params);
		FString SuperArgs = BuildParamNameList(Event.Params);

		if (FindKnownOverride(Event.Name, KnownInfo))
		{
			ReturnType = KnownInfo.ReturnType;
			ParamString = KnownInfo.Params;
			SuperArgs = KnownInfo.SuperCallArgs;
		}

		Cpp += TEXT("\n");
		Cpp += FString::Printf(TEXT("%s %s::%s(%s)\n{\n"), *ReturnType, *FullClassName, *Event.Name, *ParamString);
		Cpp += FString::Printf(TEXT("\tSuper::%s(%s);\n"), *Event.Name, *SuperArgs);
		Cpp += TEXT("}\n");
	}

	Result.CppText = Cpp;

	return Result;
}
