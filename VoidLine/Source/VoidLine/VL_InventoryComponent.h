#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_RunContract.h"
#include "VL_InventoryComponent.generated.h"

class UVL_ResourceDefinition;

DECLARE_MULTICAST_DELEGATE(FVL_OnInventoryChanged);

// The rig's cargo: what it has collected, plus the contract it is trying to fill.
UCLASS(ClassGroup = (VL), meta = (BlueprintSpawnableComponent))
class VOIDLINE_API UVL_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_InventoryComponent();

	void AddResource(UVL_ResourceDefinition* Resource, int32 Amount);
	int32 GetCount(const UVL_ResourceDefinition* Resource) const;
	void ClearCargo();

	void SetContract(UVL_RunContract* NewContract);
	const UVL_RunContract* GetContract() const { return Contract; }

	const TArray<FVL_ResourceAmount>& GetItems() const { return Items; }

	FVL_OnInventoryChanged OnChanged;

protected:
	// The contract for this run. Assign a test contract here until cities exist.
	UPROPERTY(EditAnywhere, Category = "VL|Inventory")
	TObjectPtr<UVL_RunContract> Contract;

private:
	UPROPERTY(Transient)
	TArray<FVL_ResourceAmount> Items;
};