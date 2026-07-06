#include "S_MasterField.h"

FS_MasterField::FS_MasterField()
{
}

void FS_MasterField::UserConstructionScript()
{
}

UCullingField* FS_MasterField::FalloffAndCullSwitch_Main(double Magnitude, EFieldFalloffType FalloffType, FVector2D FalloffMinMax, UOperatorField* OperatorField)
{
	return nullptr;
}

void FS_MasterField::SetVisibility()
{
}

void FS_MasterField::ForceMultiplier()
{
}

void FS_MasterField::DisplayTextSetup()
{
}

void FS_MasterField::InitializeFieldVariables()
{
}

void FS_MasterField::MakeDynamic_EnableNonGC()
{
}

UOperatorField* FS_MasterField::FalloffShapeSwitch(EFieldFalloffType falloffType, FVector2D falloffMinMax)
{
	return nullptr;
}

UNoiseField* FS_MasterField::CalculateNoise()
{
	return nullptr;
}

void FS_MasterField::CE_Trigger_Implementation()
{
}

void FS_MasterField::ReceiveBeginPlay(FString OutputDelegate)
{
	Super::ReceiveBeginPlay(OutputDelegate);
}

void FS_MasterField::ReceiveTick(FString OutputDelegate, float DeltaSeconds)
{
	Super::ReceiveTick(OutputDelegate, DeltaSeconds);
}
