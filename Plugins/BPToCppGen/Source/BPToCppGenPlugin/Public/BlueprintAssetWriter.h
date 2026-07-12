#pragma once

#include "CoreMinimal.h"
#include "BPGraphTypes.h"

struct FEdGraphPinType;

class FBlueprintAssetWriter
{
public:
	static bool CreateBlueprintAsset(const FBPClassDesc& ClassDesc, const FString& PackagePath, TArray<FString>& OutWarnings);

private:
	static FEdGraphPinType StringTypeToPinType(const FString& CppType, TArray<FString>& OutWarnings);
};
