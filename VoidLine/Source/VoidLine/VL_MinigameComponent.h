#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_MinigameBase.h"
#include "VL_MinigameComponent.generated.h"

// bSuccess, pieces caught, total pieces
DECLARE_DELEGATE_ThreeParams(FVL_OnMinigameFinished, bool, int32, int32);

// Runs whichever minigame the resource being collected asks for.
UCLASS(ClassGroup = (VL), meta = (BlueprintSpawnableComponent))
class VOIDLINE_API UVL_MinigameComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_MinigameComponent();

	// Makes a fresh copy of the template and starts it.
	void StartMinigame(const UVL_MinigameBase* Template);

	// Stops without calling OnFinished.
	void CancelMinigame();

	void SendInput(EVL_MinigameInput Input);

	bool IsRunning() const { return Current && !Current->IsFinished(); }
	const UVL_MinigameBase* GetCurrent() const { return Current; }

	FVL_OnMinigameFinished OnFinished;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UVL_MinigameBase> Current;
};