#include "VL_GhostTownGenerator.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "VL_ResourceDefinition.h"
#include "VL_ResourceNode.h"
#include "VL_WorldStateSubsystem.h"

AVL_GhostTownGenerator::AVL_GhostTownGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Buildings = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Buildings"));
	Buildings->SetupAttachment(Root);
	Buildings->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Buildings->SetStaticMesh(CubeMesh.Object);
	}
}

void AVL_GhostTownGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UVL_WorldStateSubsystem* State = GI->GetSubsystem<UVL_WorldStateSubsystem>())
		{
			State->OnNightChanged.AddDynamic(this, &AVL_GhostTownGenerator::HandleNightChanged);
		}
	}

	Generate();
}

void AVL_GhostTownGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UVL_WorldStateSubsystem* State = GI->GetSubsystem<UVL_WorldStateSubsystem>())
			{
				State->OnNightChanged.RemoveDynamic(this, &AVL_GhostTownGenerator::HandleNightChanged);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AVL_GhostTownGenerator::HandleNightChanged(int32 NewNight)
{
	Generate();
}

void AVL_GhostTownGenerator::Generate()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Seed = TownId;
	int32 Night = 0;
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UVL_WorldStateSubsystem* State = GI->GetSubsystem<UVL_WorldStateSubsystem>())
		{
			Seed = State->GetSeedFor(TownId);
			Night = State->GetNight();
		}
	}

	// Clear the previous layout.
	Buildings->ClearInstances();
	for (AActor* Node : SpawnedNodes)
	{
		if (IsValid(Node))
		{
			Node->Destroy();
		}
	}
	SpawnedNodes.Reset();

	// Everything below draws from this one stream, so the same seed gives the same town.
	FRandomStream Stream(Seed);

	const int32 Grid = FMath::Max(GridSize, 5);
	const float Half = (Grid - 1) * 0.5f;
	const float Footprint = (CellSize * 0.75f) / 100.f; // the cube mesh is 100 cm wide

	TArray<FVector> FreeCells;
	int32 BuildingCount = 0;

	for (int32 X = 0; X < Grid; ++X)
	{
		for (int32 Y = 0; Y < Grid; ++Y)
		{
			const FVector Center((X - Half) * CellSize, (Y - Half) * CellSize, 0.f);
			const bool bBorder = (X == 0 || Y == 0 || X == Grid - 1 || Y == Grid - 1);
			const bool bStreet = bBorder || (X % 3 == 0) || (Y % 3 == 0);

			bool bBuilding = false;
			if (!bStreet)
			{
				bBuilding = Stream.FRand() < BuildingChance;
			}

			if (bBuilding)
			{
				const float Height = Stream.FRandRange(MinBuildingHeight, MaxBuildingHeight);
				const FTransform T(FRotator::ZeroRotator,
					Center + FVector(0.f, 0.f, Height * 0.5f),
					FVector(Footprint, Footprint, Height / 100.f));
				Buildings->AddInstance(T);
				++BuildingCount;
			}
			else if (!bBorder)
			{
				FreeCells.Add(Center);
			}
		}
	}

	// Shuffle the open cells, then place resources in the first few.
		// Shuffle the open cells, then place resources in the first few.
	for (int32 K = FreeCells.Num() - 1; K > 0; --K)
	{
		FreeCells.Swap(K, Stream.RandRange(0, K));
	}

	TArray<int32> TypeCounts;
	TypeCounts.Init(0, ResourceTypes.Num());

	const int32 NodeCount = (ResourceTypes.Num() > 0) ? FMath::Min(ResourceCount, FreeCells.Num()) : 0;

	for (int32 K = 0; K < NodeCount; ++K)
	{
		const int32 TypeIndex = Stream.RandRange(0, ResourceTypes.Num() - 1);
		UVL_ResourceDefinition* Type = ResourceTypes[TypeIndex];
		const FVector WorldPos = GetActorTransform().TransformPosition(FreeCells[K] + FVector(0.f, 0.f, 60.f));

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AVL_ResourceNode* Node = World->SpawnActor<AVL_ResourceNode>(
			AVL_ResourceNode::StaticClass(), WorldPos, FRotator::ZeroRotator, Params))
		{
			Node->Init(Type);
			SpawnedNodes.Add(Node);
			++TypeCounts[TypeIndex];
		}
	}

	if (GEngine)
	{
		FString Counts;
		for (int32 i = 0; i < ResourceTypes.Num(); ++i)
		{
			const FString Name = ResourceTypes[i] ? ResourceTypes[i]->DisplayName.ToString() : TEXT("?");
			Counts += FString::Printf(TEXT("  %s %d"), *Name, TypeCounts[i]);
		}
		if (ResourceTypes.Num() == 0)
		{
			Counts = TEXT("  NO RESOURCE TYPES ASSIGNED");
		}

		GEngine->AddOnScreenDebugMessage(105 + TownId, 8.f, FColor::Cyan,
			FString::Printf(TEXT("Town %d | Night %d | Seed %d | Buildings %d |%s"),
				TownId, Night, Seed, BuildingCount, *Counts));
	}
}
