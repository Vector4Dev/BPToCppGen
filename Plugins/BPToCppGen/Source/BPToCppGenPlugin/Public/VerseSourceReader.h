#pragma once

#include "CoreMinimal.h"
#include "BPGraphTypes.h"

class FVerseSourceReader
{
public:
	static bool ReadVerseSource(const FString& SourceText, FBPClassDesc& OutClassDesc, TArray<FString>& OutWarnings);

private:
	static FString ReverseMapType(const FString& VerseType, TArray<FString>& OutWarnings);
	static FString PascalCaseFromSnake(const FString& SnakeName);
	static TArray<FBPParamDesc> ParseVerseParamList(const FString& ParamListText, TArray<FString>& OutWarnings);
	static int32 GetIndent(const FString& Line);
};
