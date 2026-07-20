#pragma once

#include "CoreMinimal.h"
#include "BPGraphTypes.h"

class UBlueprint;
struct FEdGraphPinType;
class UEdGraphPin;

class FBlueprintAssetReader
{
public:
	static bool ReadBlueprint(UBlueprint* Blueprint, FBPClassDesc& OutClassDesc, TArray<FString>& OutWarnings);

private:
	static FString PinTypeToString(const FEdGraphPinType& PinType);
	static FString ClassToCppName(const UClass* Class);
	static FString MakeSafeIdentifier(const FString& RawName);
};