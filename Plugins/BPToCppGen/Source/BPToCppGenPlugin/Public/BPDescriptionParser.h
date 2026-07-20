#pragma once

#include "CoreMinimal.h"
#include "BPGraphTypes.h"

enum class EBPParseSection : uint8
{
	None,
	Variables,
	Functions,
	Events
};

class FBPDescriptionParser
{
public:
	static bool Parse(const FString& SourceText, FBPClassDesc& OutClassDesc, TArray<FString>& OutErrors);
	static FString Serialize(const FBPClassDesc& ClassDesc);

private:
	static bool FindMatchingParen(const FString& Text, int32 OpenIndex, int32& OutCloseIndex);
	static bool ParseHeaderLine(const FString& Line, FBPClassDesc& OutClassDesc);
	static bool ParseVariableLine(const FString& Line, FBPVariableDesc& OutVariable);
	static bool ParseFunctionLine(const FString& Line, FBPFunctionDesc& OutFunction);
	static bool ParseEventLine(const FString& Line, FBPEventDesc& OutEvent);
	static TArray<FBPParamDesc> ParseParamList(const FString& ParamListText);
	static FString StripTrailingComma(const FString& InString);
};