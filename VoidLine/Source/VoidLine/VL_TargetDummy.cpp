#include "VL_TargetDummy.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "VL_HealthComponent.h"

AVL_TargetDummy::AVL_TargetDummy()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetRelativeScale3D(FVector(1.f, 1.f, 2.f));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	Health = CreateDefaultSubobject<UVL_HealthComponent>(TEXT("Health"));

	Tags.Add(TEXT("VL_Enemy"));
}

void AVL_TargetDummy::BeginPlay()
{
	Super::BeginPlay();

	Health->OnHealthChanged.AddDynamic(this, &AVL_TargetDummy::HandleHealthChanged);
	Health->OnDeath.AddDynamic(this, &AVL_TargetDummy::HandleDeath);
}

void AVL_TargetDummy::HandleHealthChanged(float NewHealth, float MaxHealth, float Delta)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red,
			FString::Printf(TEXT("%s health: %.0f"), *GetName(), NewHealth));
	}
}

void AVL_TargetDummy::HandleDeath()
{
	Destroy();
}