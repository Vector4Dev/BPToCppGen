#pragma once

#include "CoreMinimal.h"
#include "FieldSystemActor.h"
#include "S_MasterField.generated.h"

UCLASS()
class YOURGAME_API FS_MasterField : public AFieldSystemActor
{
	GENERATED_BODY()

public:
	FS_MasterField();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool Field Active;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	UOperatorField* OperatorFIeld_Input;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool Debug;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EEFieldActivationType ActivationType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EEFieldShapeType Field Falloff Shape;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseTick;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double DelayAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool Use External Strain;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double Strain Magnitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldFalloffType StrainFalloffType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector2D StrainFalloffMinMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int32 NumStrainHits;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseRadialVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double Radial Magnitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseDirectionalVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double DirectionalMagnitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseTorque;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double TorqueMult;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldFalloffType VelocityFieldFalloffType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector2D VelocityFalloffMinMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseNoise;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector2D NoiseMinMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseDecay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double DecayAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldFalloffType DecayFalloffType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector2D DecayFalloffMinMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	UStaticMeshComponent* FieldVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double DecayDelay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double MaxDecayAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector RadialPositionOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool OverideDIrectionalVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector DIrectionalVectorOveride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector TorqueVectorOveride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EEFieldForceVel Force/Velocity Vector Switch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double ForceMult;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool BoxCullingOnPlanar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double PlanarFalloffDistOveride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool Force Dynamic Switch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool ActivateTaggedStaticAndSkeletal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FText Chaos Field Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FLinearColor FieldColour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool ShowDebugText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool ShowWireFrame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool ShowSolidShapes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double DirectionalDisplayScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double RadialDisplayScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double Text Vertical Offset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldPhysicsType LinearPhysicsType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldPhysicsType AngularPhysicsType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldFalloffType FieldFalloffType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldFalloffType Field Falloff Noise;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldFalloffType Field Falloff Torque;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseLifespan;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double FieldLifespan;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool UseFramesForTiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double FPS;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EObjectStateTypeEnum Dynamic State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double PlanarFalloffDist;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double TotalDecay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector2D FalloffMinMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FLinearColor PlanarFalloffExtentColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector DIrectionalVelocityVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector upVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector worldLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector forwardVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector rightVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	USkeletalMeshComponent* skel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	TArray<AStaticMeshActor*> SimmableStaticMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	TArray<ASkeletalMeshActor*> SimmableSkelMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FName PulseLevel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	TArray<FText> TextDisplay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	TArray<UTextRenderComponent*> AllText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FLinearColor ArrowColour_Dir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FLinearColor ArrowColour_Normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FLinearColor DeactivatedColour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FLinearColor DeactivatedTextColour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	UMaterialInstanceDynamic* Preview Material;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double TimeElapsed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double NoiseScaleMult;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FTransform NewVar_0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool isTriggered;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double NoiseScaleBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool DestroyActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	EFieldFalloffType FieldFalloffType_Input;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	FVector2D FalloffMinMax_Input;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	double Magnitude_Input;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void UserConstructionScript();
	UFUNCTION(BlueprintCallable, Category = "Functions")
	UCullingField* FalloffAndCullSwitch_Main(double Magnitude, EFieldFalloffType FalloffType, FVector2D FalloffMinMax, UOperatorField* OperatorField);
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetVisibility();
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void ForceMultiplier();
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void DisplayTextSetup();
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeFieldVariables();
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void MakeDynamic_EnableNonGC();
	UFUNCTION(BlueprintCallable, Category = "Functions")
	UOperatorField* FalloffShapeSwitch(EFieldFalloffType falloffType, FVector2D falloffMinMax);
	UFUNCTION(BlueprintCallable, Category = "Functions")
	UNoiseField* CalculateNoise();

	UFUNCTION(BlueprintNativeEvent, Category = "Events")
	void CE_Trigger();
	virtual void CE_Trigger_Implementation();

protected:
	virtual void ReceiveBeginPlay(FString OutputDelegate) override;
	virtual void ReceiveTick(FString OutputDelegate, float DeltaSeconds) override;
};
