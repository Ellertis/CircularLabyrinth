// Fill out your copyright notice in the Description page of Project Settings.


#include "LabyMonCul.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
ALabyMonCul::ALabyMonCul()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Components Init
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	RootComponent = DefaultSceneRoot;
	
	Path_HISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>("Path_HISM");
	Path_HISM->SetupAttachment(DefaultSceneRoot);
	
	Pillars_HISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>("Pillars_HISM");
	Pillars_HISM->SetupAttachment(DefaultSceneRoot);
	
	RadialWalls_HISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>("RadialWalls_HISM");
	RadialWalls_HISM->SetupAttachment(DefaultSceneRoot);
	
	CircularWall_HISM =	CreateDefaultSubobject<UInstancedStaticMeshComponent>("CircularWall_HISM");
	CircularWall_HISM->SetupAttachment(DefaultSceneRoot);

	//Variables
	FSLabyrinthCell Cell;
}

// Called when the game starts or when spawned
void ALabyMonCul::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALabyMonCul::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ClearProperties();
}

// Called every frame
void ALabyMonCul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALabyMonCul::ClearProperties()
{
	// Clear the L_InstancedTextRenderer Array
	for (UTextRenderComponent* TextRenderer : L_InstancedTextRenderer)
	{
		if (TextRenderer)
		{
			TextRenderer->DestroyComponent();
		}
	}
	L_InstancedTextRenderer.Empty();
}

void ALabyMonCul::AddDebugTextRenderer(FVector TextLoc, FString TextMessage)
{
	if(DebugIndex)
	{
		UTextRenderComponent* NewTextRenderer = NewObject<UTextRenderComponent>(this);
		NewTextRenderer->RegisterComponent();
		NewTextRenderer->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		NewTextRenderer->SetRelativeLocation(TextLoc + GetActorLocation());
		NewTextRenderer->SetRelativeRotation(FRotator(180, 90, 0));
		NewTextRenderer->SetText(FText::FromString(TextMessage));
		NewTextRenderer->SetHorizontalAlignment(EHTA_Center);
		NewTextRenderer->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
		NewTextRenderer->SetWorldSize(100);
		L_InstancedTextRenderer.Add(NewTextRenderer);
	}
		
}

void ALabyMonCul::CalculateCellNeighbors(FSLabyrinthCell& Cell)
{
	FSLabyrinthCell NewCell = Cell;
	NewCell.Neighbors.Empty();
	if(NewCell.Index == 0 and NewCell.Ring == 0 and NewCell.Sector == 0)
	{
		for(int32 i = 0; i < GetRingSubDivision(1)-1; i++)
		{
			NewCell.Neighbors.Add(i+1);
			Cell = NewCell;
		}
	}
	else
	{
		float LRing = Cell.Ring;
		float LSector = Cell.Sector;
		float LCurrentSubdivision = GetRingSubDivision(LRing);
		float LLeftSector = FMath::Modulo((LSector - 1 + LCurrentSubdivision),LCurrentSubdivision);
		float LRightSector = FMath::Modulo((LSector + 1),LCurrentSubdivision);
		NewCell.Neighbors.Add(GetCellIndex(LRing, LLeftSector));
		NewCell.Neighbors.Add(GetCellIndex(LRing, LRightSector));
		Cell = NewCell;
	}
}

int32 ALabyMonCul::GetRingSubDivision(int32 Ring) const
{
	float FRing = static_cast<float>(Ring);
	FRing = FMath::Log2(FRing);
	int32 IRing = FMath::Floor(FRing);
	IRing = IRing + SubdivisionFactor;
	FRing = static_cast<float>(IRing);
	FRing = FMath::Pow(2.0f, FRing);
	IRing = FMath::Floor(FRing);
	return IRing;
}

int32 ALabyMonCul::GetCellIndex(int32 Ring, int32 Sector)
{
	int32 LIndex = 1;
	int32 LSubdivision = 0;
	int32 LCurrentSubdivision = 0;
	if(Ring == 0)
	{
		return 0;
	}
	for(int i = 1; i < Ring - 1; i++)
	{
		LSubdivision = GetRingSubDivision(i);
		LIndex = LSubdivision + LIndex;
	}
	LCurrentSubdivision = GetRingSubDivision(Ring);
	return (LIndex + FMath::Modulo(Sector,LCurrentSubdivision));
}
	
