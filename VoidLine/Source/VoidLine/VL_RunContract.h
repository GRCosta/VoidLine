#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VL_RunContract.generated.h"

class UVL_ResourceDefinition;

USTRUCT(BlueprintType)
struct FVL_ResourceAmount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TObjectPtr<UVL_ResourceDefinition> Resource = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0"))
	int32 Amount = 0;
};

// What a run is meant to bring back. A city quest will produce one of these later.
UCLASS(BlueprintType)
class VOIDLINE_API UVL_RunContract : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Shown above the cargo list, e.g. the city or quest name.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contract")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contract")
	TArray<FVL_ResourceAmount> Needs;
};