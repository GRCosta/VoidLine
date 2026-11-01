#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_InteractionComponent.generated.h"

class AVL_ResourceNode;

// Finds the nearest resource node in reach and runs the collection minigame on button press.
UCLASS(ClassGroup = (VL), meta = (BlueprintSpawnableComponent))
class VOIDLINE_API UVL_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_InteractionComponent();

	// Called by the rig when the interact button is pressed.
	void TryInteract();

	// Ends a collection in progress without a reward (and restores normal time).
	void CancelCollection();

	bool IsCollecting() const { return bCollecting; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// How close (cm) the rig must be to a node to collect it.
	UPROPERTY(EditAnywhere, Category = "VL|Interaction")
	float InteractRadius = 500.f;

	// World speed while collecting. 0.3 = 30% speed.
	UPROPERTY(EditAnywhere, Category = "VL|Interaction", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float CollectTimeDilation = 0.3f;

private:
	void FindFocus();
	void SetFocus(AVL_ResourceNode* NewFocus);
	void HandleMinigameFinished(bool bSuccess, int32 Hits, int32 Total);
	void FinishCollection(bool bSuccess, int32 Hits, int32 Total);

	TWeakObjectPtr<AVL_ResourceNode> Focus;
	TWeakObjectPtr<AVL_ResourceNode> Active;
	bool bCollecting = false;
	float FocusScanTimer = 0.f;
};