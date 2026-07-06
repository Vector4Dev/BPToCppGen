#pragma once

#include "CoreMinimal.h"
#include "BPGraphTypes.h"

struct FGeneratedBPCode
{
	FString HeaderFileName;
	FString CppFileName;
	FString HeaderText;
	FString CppText;
};

struct FKnownOverrideInfo
{
	FString ReturnType;
	FString Params;
	FString SuperCallArgs;
};

class FCppSkeletonGenerator
{
public:
	static FGeneratedBPCode Generate(const FBPClassDesc& ClassDesc, const FString& ApiMacro);

private:
	static FString MapType(const FString& BPType);
	static FString InferClassPrefix(const FString& ParentClass);
	static FString ApplyClassPrefix(const FString& ClassName, const FString& Prefix);
	static FString GuessParentHeader(const FString& ParentClass);
	static FString BuildParamString(const TArray<FBPParamDesc>& Params);
	static FString BuildParamNameList(const TArray<FBPParamDesc>& Params);
	static FString DefaultReturnValue(const FString& ReturnType);
	static bool FindKnownOverride(const FString& EventName, FKnownOverrideInfo& OutInfo);
};
