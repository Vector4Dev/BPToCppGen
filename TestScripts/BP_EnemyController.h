#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BP_EnemyController.generated.h"

UCLASS()
class YOURGAME_API ABP_EnemyController : public AAIController
{
	GENERATED_BODY()

public:
	ABP_EnemyController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float Health = 100.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool IsAlerted = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	AActor* TargetActor;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void TakeDamage(float Amount);
	UFUNCTION(BlueprintCallable, Category = "Functions")
	AActor* FindNearestTarget();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
