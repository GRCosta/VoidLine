#include "VL_ResourceNode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AVL_ResourceNode::AVL_ResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = Mesh;
}

void AVL_ResourceNode::Init(EVL_ResourceType InType)
{
	ResourceType = InType;

	const TCHAR* Path = TEXT("/Engine/BasicShapes/Cube.Cube");
	FVector Scale(0.7f);

	switch (InType)
	{
	case EVL_ResourceType::Fuel:      Path = TEXT("/Engine/BasicShapes/Cylinder.Cylinder"); Scale = FVector(0.6f, 0.6f, 0.9f); break;
	case EVL_ResourceType::Chemicals: Path = TEXT("/Engine/BasicShapes/Cone.Cone");         Scale = FVector(0.7f, 0.7f, 0.9f); break;
	case EVL_ResourceType::Relics:    Path = TEXT("/Engine/BasicShapes/Sphere.Sphere");     Scale = FVector(0.8f);             break;
	default: break;
	}

	if (UStaticMesh* Loaded = LoadObject<UStaticMesh>(nullptr, Path))
	{
		Mesh->SetStaticMesh(Loaded);
	}
	Mesh->SetRelativeScale3D(Scale);
}