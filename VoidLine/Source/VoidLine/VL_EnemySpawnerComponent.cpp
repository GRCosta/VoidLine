#include "VL_EnemySpawnerComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "VL_ChaserEnemy.h"

UVL_EnemySpawnerComponent::UVL_EnemySpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	EnemyClass = AVL_ChaserEnemy::StaticClass();
}

void UVL_EnemySpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartSpawning();
	}
}

void UVL_EnemySpawnerComponent::StartSpawning()
{
	if (UWorld* World = GetWorld())
	{
		// First wave after 2 seconds, then every SpawnInterval. Follows world time.
		World->GetTimerManager().SetTimer(WaveTimer, this, &UVL_EnemySpawnerComponent::SpawnWave,
			SpawnInterval, true, 2.f);
	}
}

void UVL_EnemySpawnerComponent::StopSpawning()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WaveTimer);
	}
}

void UVL_EnemySpawnerComponent::SpawnWave()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World || !EnemyClass)
	{
		return;
	}

	Alive.RemoveAll([](const TWeakObjectPtr<AActor>& Enemy) { return !Enemy.IsValid(); });

	for (int32 i = 0; i < EnemiesPerWave && Alive.Num() < MaxAlive; ++i)
	{
		// Pick an angle relative to the rig's forward direction, skipping the front cone.
		const float RelativeAngle = FMath::FRandRange(ForwardExclusionHalfAngle, 360.f - ForwardExclusionHalfAngle);
		const FVector Direction = Owner->GetActorForwardVector().GetSafeNormal2D()
			.RotateAngleAxis(RelativeAngle, FVector::UpVector);
		const float Radius = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);
		const FVector Location = Owner->GetActorLocation() + Direction * Radius;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (AVL_ChaserEnemy* Enemy = World->SpawnActor<AVL_ChaserEnemy>(
			EnemyClass, Location, FRotator::ZeroRotator, Params))
		{
			Alive.Add(Enemy);
		}
	}
}