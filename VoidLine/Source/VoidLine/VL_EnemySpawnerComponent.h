#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_EnemySpawnerComponent.generated.h"

class AVL_ChaserEnemy;

// Spawns waves of enemies in a ring around its owner (the rig).
UCLASS(ClassGroup = (VL), meta = (BlueprintSpawnableComponent))
class VOIDLINE_API UVL_EnemySpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_EnemySpawnerComponent();

	UFUNCTION(BlueprintCallable, Category = "VL|Enemies")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "VL|Enemies")
	void StopSpawning();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "VL|Enemies")
	TSubclassOf<AVL_ChaserEnemy> EnemyClass;

	// Seconds between waves.
	UPROPERTY(EditAnywhere, Category = "VL|Enemies", meta = (ClampMin = "0.2"))
	float SpawnInterval = 3.f;

	UPROPERTY(EditAnywhere, Category = "VL|Enemies", meta = (ClampMin = "1"))
	int32 EnemiesPerWave = 3;

	// Wave is skipped (or trimmed) when this many enemies are already alive.
	UPROPERTY(EditAnywhere, Category = "VL|Enemies", meta = (ClampMin = "1"))
	int32 MaxAlive = 40;

	UPROPERTY(EditAnywhere, Category = "VL|Enemies")
	float MinSpawnRadius = 1800.f;

	UPROPERTY(EditAnywhere, Category = "VL|Enemies")
	float MaxSpawnRadius = 2600.f;

	UPROPERTY(EditAnywhere, Category = "VL|Enemies")
	bool bAutoStart = true;

	// Enemies never spawn within this many degrees either side of the rig's forward direction.
	UPROPERTY(EditAnywhere, Category = "VL|Enemies", meta = (ClampMin = "0.0", ClampMax = "170.0"))
	float ForwardExclusionHalfAngle = 70.f;

private:
	void SpawnWave();

	FTimerHandle WaveTimer;
	TArray<TWeakObjectPtr<AActor>> Alive;
};