#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_GhostTownGenerator.generated.h"

class UInstancedStaticMeshComponent;

// Builds a ghost town zone from the night's seed: a street lattice with random
// blocks of buildings, plus resource nodes in the open cells.
UCLASS()
class VOIDLINE_API AVL_GhostTownGenerator : public AActor
{
	GENERATED_BODY()

public:
	AVL_GhostTownGenerator();

	// Builds (or rebuilds) the town from the current night's seed.
	void Generate();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleNightChanged(int32 NewNight);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Buildings;

	// Give each ghost town its own id so they look different on the same night.
	UPROPERTY(EditAnywhere, Category = "VL|Generation")
	int32 TownId = 0;

	UPROPERTY(EditAnywhere, Category = "VL|Generation", meta = (ClampMin = "5", ClampMax = "25"))
	int32 GridSize = 9;

	// Centimeters per cell.
	UPROPERTY(EditAnywhere, Category = "VL|Generation", meta = (ClampMin = "400"))
	float CellSize = 1200.f;

	UPROPERTY(EditAnywhere, Category = "VL|Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BuildingChance = 0.6f;

	UPROPERTY(EditAnywhere, Category = "VL|Generation")
	float MinBuildingHeight = 400.f;

	UPROPERTY(EditAnywhere, Category = "VL|Generation")
	float MaxBuildingHeight = 1400.f;

	UPROPERTY(EditAnywhere, Category = "VL|Generation", meta = (ClampMin = "0"))
	int32 ResourceCount = 8;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedNodes;
};