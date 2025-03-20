// Fill out your copyright notice in the Description page of Project Settings.


#include "LabyMonCul.h"

#include <functional>

#include "IPropertyTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Quaternion.h"
#include "CompGeom/FitOrientedBox2.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Math/TransformCalculus3D.h"

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
	GenerateGrid();
	GenerateGeometry();
	SetLabyrinthEntrance(StartPath);
	SetLabyrinthExit(EndPath);
	//FF();
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
		NewTextRenderer->SetRelativeLocation(TextLoc); //+ GetActorLocation()
		NewTextRenderer->SetRelativeRotation(FRotator(90, 0, 0));
		NewTextRenderer->SetText(FText::FromString(TextMessage));
		NewTextRenderer->SetHorizontalAlignment(EHTA_Center);
		NewTextRenderer->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
		NewTextRenderer->SetWorldSize(100);
		L_InstancedTextRenderer.Add(NewTextRenderer);
	}
		
}

void ALabyMonCul::CalculateCellNeighbors(FSLabyrinthCell& Cell)
{
	Cell.Neighbors.Empty();
	
	if(Cell.Index == 0 and Cell.Ring == 0 and Cell.Sector == 0)
	{
		for(int32 i = 0; i < GetRingSubDivision(1)-1; i++)
		{
			Cell.Neighbors.Add(i+1);
		}
	}
	else
	{
		LRing = Cell.Ring;
		LSector = Cell.Sector;
		LCurrentSubdivision = GetRingSubDivision(LRing);
		LLeftSector = FMath::Modulo((LSector - 1 + LCurrentSubdivision),LCurrentSubdivision);
		LRightSector = FMath::Modulo((LSector + 1),LCurrentSubdivision);
		Cell.Neighbors.Add(GetCellIndex(LRing, LLeftSector));
		Cell.Neighbors.Add(GetCellIndex(LRing, LRightSector));
	}
	if(LRing == 1)
	{
		Cell.Neighbors.Add(0);
	}
	if(LRing > 1)
	{
		LCurrentSubdivision = GetRingSubDivision(LRing);
		if(LCurrentSubdivision > GetRingSubDivision(LRing - 1))
		{
			Cell.Neighbors.Add(GetCellIndex(LRing - 1, static_cast<int32>(static_cast<float>(LSector) / 2)));
		}
		else
		{
			Cell.Neighbors.Add(GetCellIndex(LRing - 1, LSector));
		}
	}
	if(LRing < MaxRings - 1)
	{
		LCurrentSubdivision = GetRingSubDivision(LRing);
		if(LCurrentSubdivision < GetRingSubDivision(LRing + 1))
		{
			Cell.Neighbors.Add(GetCellIndex(LRing + 1, LSector * 2));
			Cell.Neighbors.Add(GetCellIndex(LRing + 1, LSector * 2 + 1));
		}
		else
		{
			Cell.Neighbors.Add(GetCellIndex(LRing + 1, LSector));
		}
	}
}

int32 ALabyMonCul::GetRingSubDivision(int32 Ring) const
{
	float FRing = FMath::Log2(static_cast<float>(Ring));
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

void ALabyMonCul::GenerateGrid()
{
	int32 LCellIndex = 0;
	Cells.Empty();
	Cells.Add(FSLabyrinthCell());
	LCellIndex++;
	AddDebugTextRenderer(FVector(0,0,0), FString("0"));
	for(int32 i = 1; i < MaxRings; i++)
	{
		LRing = i;
		LCurrentSubdivision = GetRingSubDivision(i);
		for(int32 k = 0; k < LCurrentSubdivision; k++)
		{
			LSector = k;
			FSLabyrinthCell NewCell;
			NewCell.Index = LCellIndex;
			NewCell.Ring = LRing;
			NewCell.Sector = LSector;
			NewCell.Location = CalculateCellLocation(LRing, LSector); 
			Cells.Add(NewCell);
			AddDebugTextRenderer(FVector(CalculateCellLocation(LRing,LSector)), FString::FromInt(LCellIndex));	
			LCellIndex++;
		}
	}
	for (FSLabyrinthCell& Cell : Cells)
	{
		CalculateCellNeighbors(Cell);
	}
}

FVector ALabyMonCul::CalculateCellLocation(int32 Ring, int32 Sector)
{
	float MiddleRadius = 0.0f;
	float FRing = static_cast<float>(Ring);
	float FSector = static_cast<float>(Sector);
	FVector ReturnLocation = FVector(0.0f,0.0f,0.0f);
	if(Ring == 0)
	{
		return ReturnLocation;
	}
	{
		MiddleRadius = ((FRing - 1) * RingSpacing) + BaseRadius + (RingSpacing * 0.5f);
		ReturnLocation = ReturnPolarToCartesian(MiddleRadius, (FSector + 0.5f) * (360.f / static_cast<float>(GetRingSubDivision(Ring))));
	}
	return ReturnLocation;
}

void ALabyMonCul::GenerateGeometry()
{
	Pillars_HISM->ClearInstances();
	RadialWalls_HISM->ClearInstances();
	CircularWall_HISM->ClearInstances();
	MaxSubdivisions = GetRingSubDivision(MaxRings);
	BaseAngleStep = 360.0f / static_cast<float>(MaxSubdivisions);
	for(int32 i = 0; i < MaxRings; i++)
	{
		LRing = i;
		LCurrentSubdivisions = GetRingSubDivision(LRing + 1);
		LSubdivisionRatio = MaxSubdivisions / LCurrentSubdivisions;
		LRadius = BaseRadius + (static_cast<float>(LRing) * RingSpacing);
		LEffectiveAngleStep = 360.0f / static_cast<float>(LCurrentSubdivisions);
		for(int32 k = 0; k < LCurrentSubdivisions; k++)
		{
			LSector = k;
			LCircularWallAngle = (static_cast<float>(LSector) + 0.5f) * static_cast<float>(LSubdivisionRatio) * BaseAngleStep;
			LPillarAngle = (static_cast<float>(LSector)) * static_cast<float>(LSubdivisionRatio) * BaseAngleStep;
			FTransform CircularInstanceTransform;
			CircularInstanceTransform.SetLocation((ReturnPolarToCartesian(LRadius,LCircularWallAngle)));
			CircularInstanceTransform.SetRotation(UE::Math::TQuat<double>(FRotator::MakeFromEuler(FVector(0.0f, 0.0f, LCircularWallAngle + 90.0f))));
			FVector MeshSize = CircularWall_HISM->GetStaticMesh()->GetBoundingBox().GetSize();
			CircularInstanceTransform.SetScale3D(UE::Math::TVector<double>(FVector((2 * LRadius) * FMath::Sin(FMath::DegreesToRadians(LEffectiveAngleStep / 2)) / (MeshSize.X * 2.0f) * 2.0f,1.0f,1.0f)));
			CircularWall_HISM->AddInstance(FTransform(CircularInstanceTransform)); //true
			FTransform PillarInstanceTransform;
			PillarInstanceTransform.SetLocation((ReturnPolarToCartesian(LRadius,LPillarAngle)));
			PillarInstanceTransform.SetRotation(UE::Math::TQuat<double>(FRotator::MakeFromEuler(FVector(0.0f, 0.0f, LPillarAngle))));
			PillarInstanceTransform.SetScale3D(UE::Math::TVector<double>(FVector(0.2f,0.2f,1.5f)));
			Pillars_HISM->AddInstance(FTransform(PillarInstanceTransform));
		}
		if(LRing < MaxRings - 1)
		{
			LNextRadius = LRadius + RingSpacing;
			for(int32 j = 0; j < LCurrentSubdivisions; j++)
			{
				LRadialWallSector = j;
				LCurrentAngle = static_cast<float>(LRadialWallSector) * static_cast<float>(LSubdivisionRatio) * BaseAngleStep;
				LNextAngle = LCurrentAngle;
				FTransform CircularWallTransform;
				FVector intermideate1 = ((ReturnPolarToCartesian(LNextRadius,LNextAngle) + ReturnPolarToCartesian(LRadius,LCurrentAngle)));
				FVector intermideate2 = (ReturnPolarToCartesian(LNextRadius,LNextAngle) - ReturnPolarToCartesian(LRadius,LCurrentAngle));
				FRotator IntermideateRotator2 = FRotationMatrix::MakeFromX(intermideate2.GetSafeNormal()).Rotator();
				FVector MeshSize = RadialWalls_HISM->GetStaticMesh()->GetBoundingBox().GetSize();
				CircularWallTransform.SetLocation((((intermideate1 * 0.5f))));
				CircularWallTransform.SetRotation(UE::Math::TQuat<double>(IntermideateRotator2));
				CircularWallTransform.SetScale3D(UE::Math::TVector<double>(intermideate2.Size() / (MeshSize.X),1.0f,1.0f));
				CircularWall_HISM->AddInstance(FTransform(CircularWallTransform));
			}
		}
	}
}

void ALabyMonCul::SetLabyrinthEntrance(EPathStartType Type)
{
	switch(Type)
	{
		case EPathStartType::Pt_Center:
			Cells[0].bCurrent = true;
			Cells[0].bVisited = true;
			break;
		case EPathStartType::Pt_Perimeter:
			PerimeterCellChosen = GetRandomPerimeterCell();
			Cells[PerimeterCellChosen].bCurrent = true;
			Cells[PerimeterCellChosen].bVisited = true;
			OpenPerimeterCell(Cells[PerimeterCellChosen]);
			break;
	}

}

void ALabyMonCul::SetLabyrinthExit(EPathEndType Type)
{
	switch(Type)
	{
		case EPathEndType::Pt_Center:
			Cells[0].bCurrent = true;
			Cells[0].bVisited = true;
			break;
	}
}

int32 ALabyMonCul::GetRandomPerimeterCell()
{
	for (FSLabyrinthCell& Cell : Cells)
	{
		if(Cell.Ring == MaxRings - 1)
		{
			PossibleIndex.Add(Cell.Index);
		}
	}
	int32 RandomInt = FMath::RandRange(0, PossibleIndex.Num() - 1);
	PerimeterCellChosen = PossibleIndex[RandomInt];
	return PerimeterCellChosen;
}

void ALabyMonCul::OpenPerimeterCell(FSLabyrinthCell& Cell)
{
	FHitResult HitResult;
	FVector Start = Cell.Location;
	FVector End = (Cell.Location.GetSafeNormal()) * RingSpacing + Start; 
	HitResult = SingleLineTrace(Start, End);
	CircularWall_HISM->RemoveInstance(HitResult.Item);
}

FHitResult ALabyMonCul::SingleLineTrace(FVector Start, FVector End)
{
	UWorld* const World = GetWorld();
	FHitResult OutHit;
	bool bHit = false;
	TArray<AActor*> ActorsToIgnore;
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = true;
	World->LineTraceSingleByChannel(OutHit,Start, End, ECC_Visibility, TraceParams);
	DrawDebugLine(World,Start,End,FColor::Red, false, 10.0f, 0, 2.0f);
	return OutHit;
}

int32 ALabyMonCul::GetCurrentCell()
{
	for (FSLabyrinthCell& Cell : Cells)
	{
		if(Cell.bCurrent == true)
		{
			CurrentCellIndex = Cell.Index;
			break;
		}
	}
	return CurrentCellIndex;
}

FNeighborResult ALabyMonCul::GetPotentialNextNeighbor(FSLabyrinthCell Cell)
{
	TArray<int32> SelectedCellNeighbors = Cells[Cell.Index].Neighbors;
	for(int32 NeighborIndex : SelectedCellNeighbors)
	{
		if(!Cells[NeighborIndex].bVisited)
		{
			FSLabyrinthCell ToAdd = Cells[NeighborIndex];
			PotentialNeighbors.Add(ToAdd);//= Cell
		}
	}
	
/*
	for (int32 NeighborIndex : SelectedCellNeighbors)
	{
		if (NeighborIndex >= 0 && NeighborIndex < Cells.Num())
		{
			if (!Cells[NeighborIndex].bVisited)
			{
				PotentialNeighbors.Add(Cells[NeighborIndex]);
			}
		}
	}
	*/
	int32 RandomInt = FMath::RandRange(0, PotentialNeighbors.Num() - 1);
	FSLabyrinthCell ChosenNeighbor = PotentialNeighbors[RandomInt];
	bool ResultBool = PotentialNeighbors.IsEmpty();
	FNeighborResult Result = {ChosenNeighbor, !ResultBool};
	return Result;
}

void ALabyMonCul::UpdatePathLocalisation(FSLabyrinthCell Cell)
{
	Path_HISM->ClearInstances();
	FTransform UpdatePathTransform;
	UpdatePathTransform.SetLocation(Cell.Location);
	UpdatePathTransform.SetRotation(UE::Math::TQuat<double>(FVector(0.0f, 0.0f, 0.0f), 0.0f));
	UpdatePathTransform.SetScale3D(FVector(1.0f));
	Path_HISM->AddInstance(UpdatePathTransform);
}

void ALabyMonCul::RemoveWall(FSLabyrinthCell Cell1, FSLabyrinthCell Cell2)
{
	FVector Start = Cell1.Location;
	FVector End = Cell2.Location;
	UWorld* const World = GetWorld();
	FHitResult OutHit;
	TArray<AActor*> ActorsToIgnore;
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = true;
	World->LineTraceSingleByChannel(OutHit,Start, End, ECC_Visibility, TraceParams);
	DrawDebugLine(World,Start,End,FColor::Red, false, 10.0f, 0, 2.0f);
	CircularWall_HISM->RemoveInstance(OutHit.Item);
}

void ALabyMonCul::UpdateCurrentOrVisitedState(int32 CellIndex, bool bCurrent, bool bVisited)
{
	Cells[CellIndex].bCurrent = bCurrent;
	Cells[CellIndex].bVisited = bVisited;
}

void ALabyMonCul::FoundLongestPathAtRing(int32 Ring)
{
	if(PathStackCells.Max() - 1 > LongestPath && NextPathCell.Ring == Ring)
	{
		LongestPath = PathStackCells.Last().Index; // maybe +-1
		LongestPathCell = Cells[NextPathCell.Index];
	}
}

void ALabyMonCul::FF()
{
	while (RecursiveBacktrackingFinished != true)
	{
		CurrentPathCell = Cells[GetCurrentCell()];
		FNeighborResult LPotentialNeighbors = GetPotentialNextNeighbor(CurrentPathCell);
		if (LPotentialNeighbors.bVisited)
		{
			NextPathCell = LPotentialNeighbors.Cell;
			UpdatePathLocalisation(NextPathCell);
			RemoveWall(CurrentPathCell,NextPathCell);
			UpdateCurrentOrVisitedState(CurrentPathCell.Index, false, true);
			UpdateCurrentOrVisitedState(NextPathCell.Index, true, true);
			PathStackCells.Add(Cells[CurrentPathCell.Index]);
			switch(EndPath)
			{
			case EPathEndType::Pt_Center:
				FoundLongestPathAtRing(1);
				break;
			case EPathEndType::Pt_Farest:
				FoundLongestPathAtRing(MaxRings); //maybe -1
				break;
			}
		}
		else
		{
			UpdateCurrentOrVisitedState(CurrentPathCell.Index, false, true);
			UpdatePathLocalisation(CurrentPathCell);
			if(!PathStackCells.IsEmpty())
			{
				CurrentPathCell = PathStackCells.Last();
				UpdateCurrentOrVisitedState(CurrentPathCell.Index, true, true);
				UpdateCurrentOrVisitedState(NextPathCell.Index, false, true);
				PathStackCells.RemoveAt(PathStackCells.Last().Index); // maybe +-1
				FF();
			}
			RecursiveBacktrackingFinished = true;
		}
	}
	switch(EndPath)
	{
		case EPathEndType::Pt_Center:
			OpenCenterCell(LongestPathCell);
			break;
		case EPathEndType::Pt_Farest:
			OpenCenterCell(LongestPathCell);
			break;
		case EPathEndType::Pt_RandomPerimeter:
			OpenCenterCell(Cells[GetCurrentCell()]);
			break;
	}
	
}

void ALabyMonCul::OpenCenterCell(FSLabyrinthCell Cell)
{
	FHitResult HitResult;
	FVector Start = Cell.Location;
	FVector End = (Cell.Location.GetSafeNormal()) * RingSpacing + Start; 
	HitResult = SingleLineTrace(Start, End);
	CircularWall_HISM->RemoveInstance(HitResult.Item);
}



