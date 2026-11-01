#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_ResourceNode.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EVL_ResourceType : uint8
{
	Scrap,
	Fuel,
	Chemicals,
	Relics
};

// Placeholder collectible. The shape shows the type: cube = Scrap, cylinder = Fuel,
// cone = Chemicals, sphere = Relics. The collection minigame hooks in here later.
UCLASS()
class VOIDLINE_API AVL_ResourceNode : public AActor
{
	GENERATED_BODY()

public:
	AVL_ResourceNode();

	void Init(EVL_ResourceType InType);

	UFUNCTION(BlueprintPure, Category = "VL|Resource")
	EVL_ResourceType GetResourceType() const { return ResourceType; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	EVL_ResourceType ResourceType = EVL_ResourceType::Scrap;
};