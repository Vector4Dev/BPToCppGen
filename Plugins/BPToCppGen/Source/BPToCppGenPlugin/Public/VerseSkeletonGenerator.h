#pragma once

#include "CoreMinimal.h"
#include "BPGraphTypes.h"

struct FGeneratedVerseCode
{
	FString FileName;
	FString VerseText;
};

class FVerseSkeletonGenerator
{
public:
	static FGeneratedVerseCode Generate(const FBPClassDesc& ClassDesc, TArray<FString>& OutWarnings);

private:
	static FString ToSnakeCase(const FString& PascalName);
	static FString StripPrefixAndLowerFirst(const FString& EngineClassName);
	static FString MapTypeToVerse(const FString& CppStyleType);
	static FString DefaultVerseValue(const FString& VerseType);
	static FString BuildParamString(const TArray<FBPParamDesc>& Params);
};