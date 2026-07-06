#pragma once

#include "CoreMinimal.h"

struct FBPParamDesc
{
	FString Name;
	FString Type;
};

struct FBPVariableDesc
{
	FString Name;
	FString Type;
	FString DefaultValue;
	bool bHasDefault = false;
};

struct FBPFunctionDesc
{
	FString Name;
	TArray<FBPParamDesc> Params;
	FString ReturnType;
};

struct FBPEventDesc
{
	FString Name;
	TArray<FBPParamDesc> Params;
	bool bOverride = false;
};

struct FBPClassDesc
{
	FString ClassName;
	FString ParentClass;
	TArray<FBPVariableDesc> Variables;
	TArray<FBPFunctionDesc> Functions;
	TArray<FBPEventDesc> Events;
};
