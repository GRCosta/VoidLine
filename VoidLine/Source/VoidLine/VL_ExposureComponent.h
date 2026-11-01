#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_ExposureComponent.generated.h"

class UVL_HealthComponent;

UENUM(BlueprintType)
enum class EVL_ExposureStage : uint8
{
	Calm,
	Uneasy,
	Critical
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVL_OnExposureStageChanged, EVL_ExposureStage, NewStage);

// The run timer and Exposure meter. Ticks with world time, so slow-motion slows it too.
UCLASS(ClassGroup = (VL), meta = (BlueprintSpawnableComponent))
class VOIDLINE_API UVL_ExposureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_ExposureComponent();

	UFUNCTION(BlueprintCallable, Category = "VL|Exposure")
	void StartRun();

	UFUNCTION(BlueprintCallable, Category = "VL|Exposure")
	void StopRun();

	// 0 to 1 over the run.
	UFUNCTION(BlueprintPure, Category = "VL|Exposure")
	float GetExposure01() const;

	UFUNCTION(BlueprintPure, Category = "VL|Exposure")
	float GetSecondsRemaining() const;

	UFUNCTION(BlueprintPure, Category = "VL|Exposure")
	EVL_ExposureStage GetStage() const { return Stage; }

	UPROPERTY(BlueprintAssignable, Category = "VL|Exposure")
	FVL_OnExposureStageChanged OnStageChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Seconds. 1200 = 20 minutes. Use something short while testing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Exposure", meta = (ClampMin = "1.0"))
	float RunDuration = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Exposure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UneasyThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Exposure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CriticalThreshold = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VL|Exposure", meta = (ClampMin = "0.0"))
	float CriticalDamagePerSecond = 2.f;

	UPROPERTY(EditAnywhere, Category = "VL|Exposure")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, Category = "VL|Exposure")
	bool bShowDebugText = true;

private:
	void SetStage(EVL_ExposureStage NewStage);

	UPROPERTY()
	TObjectPtr<UVL_HealthComponent> Health;

	EVL_ExposureStage Stage = EVL_ExposureStage::Calm;
	float Elapsed = 0.f;
	bool bRunning = false;
};