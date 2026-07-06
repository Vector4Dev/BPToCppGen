#include "BPDescriptionParser.h"

FString FBPDescriptionParser::StripTrailingComma(const FString& InString)
{
	FString Result = InString.TrimStartAndEnd();
	if (Result.EndsWith(TEXT(",")))
	{
		Result.LeftChopInline(1);
		Result.TrimEndInline();
	}
	return Result;
}

TArray<FBPParamDesc> FBPDescriptionParser::ParseParamList(const FString& ParamListText)
{
	TArray<FBPParamDesc> Result;
	FString Trimmed = ParamListText.TrimStartAndEnd();
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
			Param.Type = ParamType;
			Result.Add(Param);
		}
	}

	return Result;
}

bool FBPDescriptionParser::ParseHeaderLine(const FString& Line, FBPClassDesc& OutClassDesc)
{
	FString Key;
	FString Value;
	if (!Line.Split(TEXT(":"), &Key, &Value))
	{
		return false;
	}

	Key.TrimStartAndEndInline();
	Value.TrimStartAndEndInline();

	if (Key.Equals(TEXT("Class"), ESearchCase::IgnoreCase))
	{
		OutClassDesc.ClassName = Value;
		return true;
	}
	if (Key.Equals(TEXT("Parent"), ESearchCase::IgnoreCase))
	{
		OutClassDesc.ParentClass = Value;
		return true;
	}

	return false;
}

bool FBPDescriptionParser::ParseVariableLine(const FString& Line, FBPVariableDesc& OutVariable)
{
	FString NamePart;
	FString Rest;
	if (!Line.Split(TEXT("("), &NamePart, &Rest))
	{
		return false;
	}

	FString TypePart;
	FString AfterType;
	if (!Rest.Split(TEXT(")"), &TypePart, &AfterType))
	{
		return false;
	}

	OutVariable.Name = NamePart.TrimStartAndEnd();
	OutVariable.Type = TypePart.TrimStartAndEnd();

	AfterType.TrimStartAndEndInline();
	if (AfterType.StartsWith(TEXT("=")))
	{
		OutVariable.DefaultValue = AfterType.RightChop(1).TrimStartAndEnd();
		OutVariable.bHasDefault = !OutVariable.DefaultValue.IsEmpty();
	}

	return !OutVariable.Name.IsEmpty() && !OutVariable.Type.IsEmpty();
}

bool FBPDescriptionParser::ParseFunctionLine(const FString& Line, FBPFunctionDesc& OutFunction)
{
	FString NamePart;
	FString Rest;
	if (!Line.Split(TEXT("("), &NamePart, &Rest))
	{
		return false;
	}

	FString ParamsPart;
	FString AfterParams;
	if (!Rest.Split(TEXT(")"), &ParamsPart, &AfterParams))
	{
		return false;
	}

	OutFunction.Name = NamePart.TrimStartAndEnd();
	OutFunction.Params = ParseParamList(ParamsPart);

	AfterParams.TrimStartAndEndInline();
	if (AfterParams.StartsWith(TEXT("->")))
	{
		OutFunction.ReturnType = AfterParams.RightChop(2).TrimStartAndEnd();
	}
	else
	{
		OutFunction.ReturnType = TEXT("void");
	}

	return !OutFunction.Name.IsEmpty();
}

bool FBPDescriptionParser::ParseEventLine(const FString& Line, FBPEventDesc& OutEvent)
{
	FString NamePart;
	FString Rest;
	if (!Line.Split(TEXT("("), &NamePart, &Rest))
	{
		OutEvent.Name = Line.TrimStartAndEnd();
		OutEvent.bOverride = false;
		return !OutEvent.Name.IsEmpty();
	}

	FString ModifierPart;
	FString AfterModifiers;
	if (!Rest.Split(TEXT(")"), &ModifierPart, &AfterModifiers))
	{
		return false;
	}

	OutEvent.Name = NamePart.TrimStartAndEnd();

	TArray<FString> Tokens;
	ModifierPart.ParseIntoArray(Tokens, TEXT(","), true);

	FString RemainingParams;
	for (int32 Index = 0; Index < Tokens.Num(); ++Index)
	{
		FString Token = Tokens[Index].TrimStartAndEnd();
		if (Index == 0 && Token.Equals(TEXT("override"), ESearchCase::IgnoreCase))
		{
			OutEvent.bOverride = true;
			continue;
		}
		RemainingParams += Token;
		if (Index != Tokens.Num() - 1)
		{
			RemainingParams += TEXT(", ");
		}
	}

	OutEvent.Params = ParseParamList(RemainingParams);

	return !OutEvent.Name.IsEmpty();
}

FString FBPDescriptionParser::Serialize(const FBPClassDesc& ClassDesc)
{
	FString Result;
	Result += FString::Printf(TEXT("Class: %s\n"), *ClassDesc.ClassName);
	Result += FString::Printf(TEXT("Parent: %s\n"), *ClassDesc.ParentClass);

	if (ClassDesc.Variables.Num() > 0)
	{
		Result += TEXT("\nVariables:\n");
		for (const FBPVariableDesc& Variable : ClassDesc.Variables)
		{
			if (Variable.bHasDefault)
			{
				Result += FString::Printf(TEXT("  %s (%s) = %s\n"), *Variable.Name, *Variable.Type, *Variable.DefaultValue);
			}
			else
			{
				Result += FString::Printf(TEXT("  %s (%s)\n"), *Variable.Name, *Variable.Type);
			}
		}
	}

	if (ClassDesc.Functions.Num() > 0)
	{
		Result += TEXT("\nFunctions:\n");
		for (const FBPFunctionDesc& Function : ClassDesc.Functions)
		{
			TArray<FString> ParamStrings;
			for (const FBPParamDesc& Param : Function.Params)
			{
				ParamStrings.Add(FString::Printf(TEXT("%s: %s"), *Param.Name, *Param.Type));
			}
			Result += FString::Printf(TEXT("  %s(%s) -> %s\n"), *Function.Name, *FString::Join(ParamStrings, TEXT(", ")), *Function.ReturnType);
		}
	}

	if (ClassDesc.Events.Num() > 0)
	{
		Result += TEXT("\nEvents:\n");
		for (const FBPEventDesc& Event : ClassDesc.Events)
		{
			TArray<FString> Tokens;
			if (Event.bOverride)
			{
				Tokens.Add(TEXT("override"));
			}
			for (const FBPParamDesc& Param : Event.Params)
			{
				Tokens.Add(FString::Printf(TEXT("%s: %s"), *Param.Name, *Param.Type));
			}

			if (Tokens.Num() > 0)
			{
				Result += FString::Printf(TEXT("  %s (%s)\n"), *Event.Name, *FString::Join(Tokens, TEXT(", ")));
			}
			else
			{
				Result += FString::Printf(TEXT("  %s\n"), *Event.Name);
			}
		}
	}

	return Result;
}

bool FBPDescriptionParser::Parse(const FString& SourceText, FBPClassDesc& OutClassDesc, TArray<FString>& OutErrors)
{
	TArray<FString> Lines;
	SourceText.ParseIntoArrayLines(Lines, true);

	EBPParseSection CurrentSection = EBPParseSection::None;

	for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
	{
		FString Line = Lines[LineIndex].TrimStartAndEnd();
		if (Line.IsEmpty())
		{
			continue;
		}

		FString SectionCandidate = Line;
		SectionCandidate.RemoveFromEnd(TEXT(":"));

		if (SectionCandidate.Equals(TEXT("Variables"), ESearchCase::IgnoreCase))
		{
			CurrentSection = EBPParseSection::Variables;
			continue;
		}
		if (SectionCandidate.Equals(TEXT("Functions"), ESearchCase::IgnoreCase))
		{
			CurrentSection = EBPParseSection::Functions;
			continue;
		}
		if (SectionCandidate.Equals(TEXT("Events"), ESearchCase::IgnoreCase))
		{
			CurrentSection = EBPParseSection::Events;
			continue;
		}

		if (ParseHeaderLine(Line, OutClassDesc))
		{
			continue;
		}

		switch (CurrentSection)
		{
		case EBPParseSection::Variables:
		{
			FBPVariableDesc Variable;
			if (ParseVariableLine(Line, Variable))
			{
				OutClassDesc.Variables.Add(Variable);
			}
			else
			{
				OutErrors.Add(FString::Printf(TEXT("Could not parse variable line %d: %s"), LineIndex + 1, *Line));
			}
			break;
		}
		case EBPParseSection::Functions:
		{
			FBPFunctionDesc Function;
			if (ParseFunctionLine(Line, Function))
			{
				OutClassDesc.Functions.Add(Function);
			}
			else
			{
				OutErrors.Add(FString::Printf(TEXT("Could not parse function line %d: %s"), LineIndex + 1, *Line));
			}
			break;
		}
		case EBPParseSection::Events:
		{
			FBPEventDesc Event;
			if (ParseEventLine(Line, Event))
			{
				OutClassDesc.Events.Add(Event);
			}
			else
			{
				OutErrors.Add(FString::Printf(TEXT("Could not parse event line %d: %s"), LineIndex + 1, *Line));
			}
			break;
		}
		default:
			OutErrors.Add(FString::Printf(TEXT("Unrecognized line %d outside any section: %s"), LineIndex + 1, *Line));
			break;
		}
	}

	if (OutClassDesc.ClassName.IsEmpty())
	{
		OutErrors.Add(TEXT("Missing 'Class:' line"));
	}
	if (OutClassDesc.ParentClass.IsEmpty())
	{
		OutErrors.Add(TEXT("Missing 'Parent:' line"));
	}

	return OutClassDesc.ClassName.Len() > 0 && OutClassDesc.ParentClass.Len() > 0;
}