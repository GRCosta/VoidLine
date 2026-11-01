#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_ResourceNode.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UVL_ResourceDefinition;

// A collectible in the world, set up from a resource data asset.
// A floating label appears when the player is near.
UCLASS()
class VOIDLINE_API AVL_ResourceNode : public AActor
{
	GENERATED_BODY()

public:
	AVL_ResourceNode();

	virtual void Tick(float DeltaSeconds) override;

	void Init(UVL_ResourceDefinition* InDefinition);

	// The nearest node in reach shows a button prompt on its label.
	void SetFocused(bool bInFocused);

	UVL_ResourceDefinition* GetDefinition() const { return Definition; }
	FString GetDisplayName() const { return DisplayName; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> Label;

	UPROPERTY()
	TObjectPtr<UVL_ResourceDefinition> Definition;

	// The label is visible when the player is within this distance (cm).
	UPROPERTY(EditAnywhere, Category = "VL|Resource")
	float IndicatorRadius = 2500.f;

private:
	void SetIndicatorVisible(bool bVisible);
	void UpdateLabel();

	FString DisplayName;
	bool bFocused = false;

	TWeakObjectPtr<AActor> Player;
	bool bIndicatorShown = false;
};