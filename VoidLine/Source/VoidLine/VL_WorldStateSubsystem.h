#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VL_WorldStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVL_OnNightChanged, int32, NewNight);

// Lives for the whole game session (survives level loads). Holds the night counter.
UCLASS()
class VOIDLINE_API UVL_WorldStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "VL|World")
	int32 GetNight() const { return Night; }

	UFUNCTION(BlueprintCallable, Category = "VL|World")
	void AdvanceNight();

	UFUNCTION(BlueprintCallable, Category = "VL|World")
	void SetNight(int32 NewNight);

	// Deterministic seed for the current night. Salt separates things (e.g. one ghost town id each).
	UFUNCTION(BlueprintPure, Category = "VL|World")
	int32 GetSeedFor(int32 Salt) const;

	UPROPERTY(BlueprintAssignable, Category = "VL|World")
	FVL_OnNightChanged OnNightChanged;

private:
	int32 Night = 1;

	// Fixed for now. Later this is randomized per new game and stored in the save.
	int32 WorldSeed = 20261031;
};