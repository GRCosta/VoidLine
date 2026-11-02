#include "VL_ResourceNode.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "VL_ResourceDefinition.h"

AVL_ResourceNode::AVL_ResourceNode()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Attached to the root (not the mesh) so it doesn't inherit the mesh's scale.
	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Root);
	Label->SetRelativeLocation(FVector(0.f, 0.f, 160.f));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextCenter);
	Label->SetWorldSize(120.f);
	Label->SetTextRenderColor(FColor(255, 220, 80));
	Label->SetVisibility(false);
}

void AVL_ResourceNode::Init(UVL_ResourceDefinition* InDefinition)
{
	Definition = InDefinition;

	UStaticMesh* Shape = nullptr;
	FVector Scale(0.7f);
	FString Name = TEXT("Resource");

	if (InDefinition)
	{
		Shape = InDefinition->Mesh.LoadSynchronous();
		Scale = InDefinition->MeshScale;
		Name = InDefinition->DisplayName.ToString();
	}

	if (!Shape)
	{
		Shape = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}
	if (Shape)
	{
		Mesh->SetStaticMesh(Shape);
	}
	Mesh->SetRelativeScale3D(Scale);

	DisplayName = Name;
	UpdateLabel();
}

void AVL_ResourceNode::SetFocused(bool bInFocused, bool bInCanCollect)
{
	if (bFocused == bInFocused && bCanCollect == bInCanCollect)
	{
		return;
	}

	bFocused = bInFocused;
	bCanCollect = bInCanCollect;
	UpdateLabel();
}

void AVL_ResourceNode::UpdateLabel()
{
	FString Text = DisplayName;

	if (bFocused)
	{
		if (bCanCollect)
		{
			Text += TEXT("\n[X] Collect");
		}
		else
		{
			Text += FString::Printf(TEXT("\nNeeds %s collector"), *DisplayName);
		}
	}

	Label->SetText(FText::FromString(Text));
	Label->SetTextRenderColor((bFocused && !bCanCollect) ? FColor(255, 90, 90) : FColor(255, 220, 80));
}

void AVL_ResourceNode::SetIndicatorVisible(bool bVisible)
{
	bIndicatorShown = bVisible;
	Label->SetVisibility(bVisible);
}

void AVL_ResourceNode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AActor* PlayerActor = Player.Get();
	if (!PlayerActor)
	{
		Player = UGameplayStatics::GetPlayerPawn(this, 0);
		return;
	}

	const bool bNear = FVector::DistSquared(PlayerActor->GetActorLocation(), GetActorLocation())
		<= FMath::Square(IndicatorRadius);

	if (bNear != bIndicatorShown)
	{
		SetIndicatorVisible(bNear);
	}

	// Turn the label to face the camera.
	if (bIndicatorShown)
	{
		if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			Label->SetWorldRotation((Camera->GetCameraLocation() - Label->GetComponentLocation()).Rotation());
		}
	}
}