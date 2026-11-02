#include "VL_InventoryComponent.h"

#include "VL_ResourceDefinition.h"

UVL_InventoryComponent::UVL_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVL_InventoryComponent::AddResource(UVL_ResourceDefinition* Resource, int32 Amount)
{
	if (!Resource || Amount <= 0)
	{
		return;
	}

	for (FVL_ResourceAmount& Item : Items)
	{
		if (Item.Resource.Get() == Resource)
		{
			Item.Amount += Amount;
			OnChanged.Broadcast();
			return;
		}
	}

	FVL_ResourceAmount NewItem;
	NewItem.Resource = Resource;
	NewItem.Amount = Amount;
	Items.Add(NewItem);
	OnChanged.Broadcast();
}

int32 UVL_InventoryComponent::GetCount(const UVL_ResourceDefinition* Resource) const
{
	for (const FVL_ResourceAmount& Item : Items)
	{
		if (Item.Resource.Get() == Resource)
		{
			return Item.Amount;
		}
	}
	return 0;
}

void UVL_InventoryComponent::ClearCargo()
{
	Items.Reset();
	OnChanged.Broadcast();
}

void UVL_InventoryComponent::SetContract(UVL_RunContract* NewContract)
{
	Contract = NewContract;
	OnChanged.Broadcast();
}