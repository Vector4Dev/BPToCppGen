#include "VerseSourceReader.h"

int32 FVerseSourceReader::GetIndent(const FString& Line)
{
	int32 Count = 0;
	for (int32 Index = 0; Index < Line.Len(); ++Index)
	{
		if (Line[Index] == TEXT(' ') || Line[Index] == TEXT('\t'))
		{
			++Count;
		}
		else
		{
			break;
		}
	}
	return Count;
}

FString FVerseSourceReader::PascalCaseFromSnake(const FString& SnakeName)
{
	TArray<FString> Tokens;
	SnakeName.ParseIntoArray(Tokens, TEXT("_"), true);

	FString Result;
	for (const FString& Token : Tokens)
	{
		if (Token.IsEmpty())
		{
			continue;
		}
		Result += Token.Left(1).ToUpper() + Token.RightChop(1).ToLower();
	}
	return Result;
}

FString FVerseSourceReader::ReverseMapType(const FString& VerseType, TArray<FString>& OutWarnings)
{
	const FString Type = VerseType.TrimStartAndEnd();

	static const TMap<FString, FString> DirectMap =
	{
		{ TEXT("float"), TEXT("float") },
		{ TEXT("int"), TEXT("int32") },
		{ TEXT("logic"), TEXT("bool") },
		{ TEXT("string"), TEXT("FString") },
		{ TEXT("void"), TEXT("void") }
	};

	if (const FString* Direct = DirectMap.Find(Type))
	{
		return *Direct;
	}

	if (Type.StartsWith(TEXT("[]")))
	{
		return FString::Printf(TEXT("TArray<%s>"), *ReverseMapType(Type.RightChop(2), OutWarnings));
	}

	if (Type.StartsWith(TEXT("[")))
	{
		FString KeyPart;
		FString ValuePart;
		if (Type.Split(TEXT("]"), &KeyPart, &ValuePart))
		{
			KeyPart = KeyPart.RightChop(1);
			return FString::Printf(TEXT("TMap<%s, %s>"), *ReverseMapType(KeyPart, OutWarnings), *ReverseMapType(ValuePart, OutWarnings));
		}
	}

	if (Type.StartsWith(TEXT("subclass(")) && Type.EndsWith(TEXT(")")))
	{
		const FString Inner = Type.Mid(9, Type.Len() - 10);
		return FString::Printf(TEXT("TSubclassOf<%s>"), *ReverseMapType(Inner, OutWarnings));
	}

	if (Type.StartsWith(TEXT("?")))
	{
		OutWarnings.Add(FString::Printf(TEXT("Reference type '%s' reconstructed as a best-effort AActor* guess, verify manually"), *Type));
		return FString::Printf(TEXT("A%s*"), *PascalCaseFromSnake(Type.RightChop(1)));
	}

	OutWarnings.Add(FString::Printf(TEXT("Unrecognized Verse type '%s' guessed as a struct, verify manually"), *Type));
	return FString::Printf(TEXT("F%s"), *PascalCaseFromSnake(Type));
}

TArray<FBPParamDesc> FVerseSourceReader::ParseVerseParamList(const FString& ParamListText, TArray<FString>& OutWarnings)
{
	TArray<FBPParamDesc> Result;
	const FString Trimmed = ParamListText.TrimStartAndEnd();
	if (Trimmed.IsEmpty())
	{
		return Result;
	}

	TArray<FString> Chunks;
	Trimmed.ParseIntoArray(Chunks, TEXT(","), true);

	for (FString Chunk : Chunks)
	{
		Chunk = Chunk.TrimStartAndEnd();
		if (Chunk.IsEmpty())
		{
			continue;
		}

		FString ParamName;
		FString ParamType;
		if (Chunk.Split(TEXT(":"), &ParamName, &ParamType))
		{
			ParamName.TrimStartAndEndInline();
			ParamType.TrimStartAndEndInline();

			FBPParamDesc Param;
			Param.Name = ParamName;
			Param.Type = ReverseMapType(ParamType, OutWarnings);
			Result.Add(Param);
		}
	}

	return Result;
}

bool FVerseSourceReader::ReadVerseSource(const FString& SourceText, FBPClassDesc& OutClassDesc, TArray<FString>& OutWarnings)
{
	TArray<FString> Lines;
	SourceText.ParseIntoArrayLines(Lines, true);

	int32 ClassLineIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Lines.Num(); ++Index)
	{
		if (Lines[Index].Contains(TEXT(" := class")))
		{
			ClassLineIndex = Index;
			break;
		}
	}

	if (ClassLineIndex == INDEX_NONE)
	{
		OutWarnings.Add(TEXT("Could not find a ':= class' declaration in the pasted Verse source"));
		return false;
	}

	FString ClassLine = Lines[ClassLineIndex].TrimStartAndEnd();
	FString ClassIdentifier;
	FString AfterClassKeyword;
	ClassLine.Split(TEXT(" := class"), &ClassIdentifier, &AfterClassKeyword);
	ClassIdentifier.TrimStartAndEndInline();

	FString ParentText;
	FString OpenParen;
	FString CloseParenRest;
	if (AfterClassKeyword.Split(TEXT("("), &OpenParen, &CloseParenRest))
	{
		FString Inner;
		FString AfterParen;
		if (CloseParenRest.Split(TEXT(")"), &Inner, &AfterParen))
		{
			ParentText = Inner.TrimStartAndEnd();
		}
	}

	OutClassDesc.ClassName = PascalCaseFromSnake(ClassIdentifier);
	OutClassDesc.ParentClass = TEXT("AActor");
	OutWarnings.Add(FString::Printf(TEXT("Parent class could not be recovered from Verse (source said '%s'); defaulted to AActor, correct this before compiling"), ParentText.IsEmpty() ? TEXT("unknown") : *ParentText));

	bool bFoundAnyPlainMethod = false;

	for (int32 Index = ClassLineIndex + 1; Index < Lines.Num(); ++Index)
	{
		const FString Trimmed = Lines[Index].TrimStartAndEnd();
		if (Trimmed.IsEmpty() || Trimmed.Equals(TEXT("@editable")))
		{
			continue;
		}

		if (Trimmed.StartsWith(TEXT("var ")))
		{
			FString Rest = Trimmed.RightChop(4);
			FString NamePart;
			FString TypeAndDefault;
			if (!Rest.Split(TEXT(":"), &NamePart, &TypeAndDefault))
			{
				continue;
			}

			FString TypePart;
			FString DefaultPart;
			const bool bHasDefault = TypeAndDefault.Split(TEXT("="), &TypePart, &DefaultPart);
			if (!bHasDefault)
			{
				TypePart = TypeAndDefault;
			}

			FBPVariableDesc Variable;
			Variable.Name = NamePart.TrimStartAndEnd();
			Variable.Type = ReverseMapType(TypePart, OutWarnings);
			if (bHasDefault)
			{
				Variable.DefaultValue = DefaultPart.TrimStartAndEnd();
				Variable.bHasDefault = !Variable.DefaultValue.IsEmpty();
			}
			OutClassDesc.Variables.Add(Variable);
			continue;
		}

		if (Trimmed.StartsWith(TEXT("OnBegin<override>")))
		{
			const int32 DeclIndent = GetIndent(Lines[Index]);
			FString Body;
			int32 EndIndex = Index + 1;
			for (; EndIndex < Lines.Num(); ++EndIndex)
			{
				const FString BodyLineTrimmed = Lines[EndIndex].TrimStartAndEnd();
				if (BodyLineTrimmed.IsEmpty())
				{
					continue;
				}
				if (GetIndent(Lines[EndIndex]) <= DeclIndent)
				{
					break;
				}
				Body += BodyLineTrimmed + TEXT("\n");
			}
			Index = EndIndex - 1;

			FBPEventDesc Event;
			Event.bOverride = true;
			if (Body.Contains(TEXT("loop:")))
			{
				Event.Name = TEXT("Tick");
				FBPParamDesc Param;
				Param.Name = TEXT("DeltaTime");
				Param.Type = TEXT("float");
				Event.Params.Add(Param);
			}
			else
			{
				Event.Name = TEXT("BeginPlay");
			}
			OutClassDesc.Events.Add(Event);
			continue;
		}

		if (Trimmed.Contains(TEXT("(")) && Trimmed.Contains(TEXT(")")) && Trimmed.Contains(TEXT(":")) && Trimmed.EndsWith(TEXT("=")))
		{
			FString NamePart;
			FString Rest;
			if (!Trimmed.Split(TEXT("("), &NamePart, &Rest))
			{
				continue;
			}

			FString ParamsPart;
			FString AfterParams;
			if (!Rest.Split(TEXT(")"), &ParamsPart, &AfterParams))
			{
				continue;
			}

			AfterParams.TrimStartAndEndInline();
			if (AfterParams.StartsWith(TEXT(":")))
			{
				AfterParams = AfterParams.RightChop(1);
			}
			AfterParams.TrimStartAndEndInline();

			FString ReturnTypeVerse = AfterParams;
			if (ReturnTypeVerse.EndsWith(TEXT("=")))
			{
				ReturnTypeVerse = ReturnTypeVerse.LeftChop(1).TrimEnd();
			}

			FBPFunctionDesc Function;
			Function.Name = NamePart.TrimStartAndEnd();
			Function.Params = ParseVerseParamList(ParamsPart, OutWarnings);
			Function.ReturnType = ReverseMapType(ReturnTypeVerse, OutWarnings);
			OutClassDesc.Functions.Add(Function);
			bFoundAnyPlainMethod = true;

			const int32 DeclIndent = GetIndent(Lines[Index]);
			int32 EndIndex = Index + 1;
			for (; EndIndex < Lines.Num(); ++EndIndex)
			{
				const FString BodyLineTrimmed = Lines[EndIndex].TrimStartAndEnd();
				if (BodyLineTrimmed.IsEmpty())
				{
					continue;
				}
				if (GetIndent(Lines[EndIndex]) <= DeclIndent)
				{
					break;
				}
			}
			Index = EndIndex - 1;
		}
	}

	if (bFoundAnyPlainMethod)
	{
		OutWarnings.Add(TEXT("Verse cannot distinguish BP Functions from non-override custom Events; all plain methods were read back as Functions, reclassify manually if any were originally Events"));
	}

	return true;
}
