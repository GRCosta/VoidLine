#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VL_MinigameBase.h"
#include "VL_ResourceDefinition.generated.h"

class UStaticMesh;

// One per resource type. Create these in the Content Browser.
UCLASS(BlueprintType)
class VOIDLINE_API UVL_ResourceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	FText DisplayName;

	// Placeholder shape. In the picker, turn on Show Engine Content and look in BasicShapes.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	FVector MeshScale = FVector(0.7f);

	// The minigame played to collect this resource, with its settings.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Resource")
	TObjectPtr<UVL_MinigameBase> Minigame;
};