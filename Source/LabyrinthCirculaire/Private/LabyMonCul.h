// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include <tuple>
#include "LabyMonCul.generated.h"

//Struct
USTRUCT(BlueprintType)
struct FSLabyrinthCell
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ring = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Sector = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector(0,0,0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Neighbors = TArray<int32>();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCurrent = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisited = false;
	
};

USTRUCT(BlueprintType)
struct FNeighborResult
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSLabyrinthCell Cell;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisited;
};

UENUM(BlueprintType)
enum class EPathStartType : uint8
{
	Pt_Center		UMETA(DisplayName = "Center"),
	Pt_Perimeter	UMETA(DisplayName = "Perimeter")
};

UENUM(BlueprintType)
enum class EPathEndType : uint8
{
	Pt_Center			UMETA(DisplayName = "Center"),
	Pt_Farest			UMETA(DisplayName = "Farest"),
	Pt_RandomPerimeter	UMETA(DisplayName = "RandomPerimeter")
};

UCLASS()
class ALabyMonCul : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALabyMonCul();

protected:
	virtual void BeginPlay() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ClearProperties();

	UFUNCTION(BlueprintCallable)
	void AddDebugTextRenderer(FVector TextLoc, FString TextMessage);
	
	UFUNCTION(BlueprintCallable)
	void CalculateCellNeighbors(FSLabyrinthCell& Cell);

	UFUNCTION(BlueprintPure)
	int32 GetRingSubDivision(int32 Ring) const;

	UFUNCTION(BlueprintPure)
	int32 GetCellIndex(int32 Ring, int32 Sector);

	UFUNCTION(BlueprintCallable)
	void GenerateGrid();

	UFUNCTION(BlueprintPure)
	FVector CalculateCellLocation(int32 Ring, int32 Sector);

	static FVector ReturnPolarToCartesian(float Radius, float Angle)
	{
		float ValueX = Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
		float ValueY = Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
		float ValueZ = 0.0f;
		return FVector(ValueX, ValueY, ValueZ);
	}
	
	UFUNCTION(BlueprintCallable)
	void GenerateGeometry();

	UFUNCTION(BlueprintCallable)
	void SetLabyrinthEntrance(EPathStartType Type);

	UFUNCTION(BlueprintCallable)
	void SetLabyrinthExit(EPathEndType Type);

	UFUNCTION(BlueprintCallable)
	int32 GetRandomPerimeterCell();

	UFUNCTION(BlueprintCallable)
	void OpenPerimeterCell(FSLabyrinthCell& Cell);

	UFUNCTION(BlueprintCallable)
	FHitResult SingleLineTrace(FVector Start, FVector End);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentCell();

	UFUNCTION(BlueprintCallable)
	FNeighborResult GetPotentialNextNeighbor(FSLabyrinthCell Cell);

	UFUNCTION(BlueprintCallable)
	void UpdatePathLocalisation(FSLabyrinthCell Cell);

	UFUNCTION(BlueprintCallable)
	void RemoveWall(FSLabyrinthCell Cell1, FSLabyrinthCell Cell2);

	UFUNCTION(BlueprintCallable)
	void UpdateCurrentOrVisitedState(int32 CellIndex, bool bCurrent, bool bVisited);

	UFUNCTION(BlueprintCallable)
	void FoundLongestPathAtRing(int32 Ring);

	UFUNCTION(BlueprintCallable)
	void FF();

	UFUNCTION(BlueprintCallable)
	void OpenCenterCell(FSLabyrinthCell Cell);
	
	//Components
	UPROPERTY(VisibleAnywhere)
	USceneComponent* DefaultSceneRoot;
	
	UPROPERTY(EditAnywhere)
	UInstancedStaticMeshComponent* Path_HISM;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* Pillars_HISM;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* RadialWalls_HISM;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* CircularWall_HISM;
	
	
	//Variables
	UPROPERTY(EditAnywhere)
	int32 MaxRings = 3;

	UPROPERTY(EditAnywhere)
	int32 SubdivisionFactor = 3;

	UPROPERTY(EditAnywhere)
	float BaseRadius = 500;

	UPROPERTY(EditAnywhere)
	float RingSpacing = 500;

	UPROPERTY(EditAnywhere)
	float MeshLength = 100;

	UPROPERTY(VisibleAnywhere)
	TArray<FSLabyrinthCell> Cells;

	UPROPERTY(VisibleAnywhere)
	TArray<FSLabyrinthCell> PathStackCells;

	UPROPERTY(VisibleAnywhere)
	TArray<UTextRenderComponent*> L_InstancedTextRenderer;

	UPROPERTY(VisibleAnywhere)
	FSLabyrinthCell CurrentPathCell;

	UPROPERTY(VisibleAnywhere)
	FSLabyrinthCell NextPathCell;

	UPROPERTY(EditAnywhere)
	bool RecursiveBacktrackingFinished = false;

	UPROPERTY(EditAnywhere)
	bool DebugIndex = true;

	UPROPERTY(VisibleAnywhere)
	int32 LongestPath = 0;

	UPROPERTY(VisibleAnywhere)
	FSLabyrinthCell LongestPathCell;

	UPROPERTY(EditAnywhere)
	EPathStartType StartPath = EPathStartType::Pt_Center;

	UPROPERTY(EditAnywhere)
	EPathEndType EndPath = EPathEndType::Pt_Farest;

	UPROPERTY(EditAnywhere)
	FRandomStream Seed = 0;

	UPROPERTY(EditAnywhere)
	float AnimationDelay = 0;

	int32 MaxSubdivisions = 0;
	float BaseAngleStep = 0.0f;
	int32 LRing = 0;
	int32 LCurrentSubdivisions = 0;
	int32 LSubdivisionRatio = 0;
	float LRadius = 0.0f;
	float LEffectiveAngleStep = 0.0f;
	int32 LSector = 0;
	float LCircularWallAngle = 0.0f;
	float LPillarAngle = 0.0f;
	float LNextRadius = 0.0f;
	int32 LRadialWallSector = 0;
	float LCurrentAngle = 0.0f;
	float LNextAngle = 0.0f;
	int32 LCurrentSubdivision = 0;
	int32 LLeftSector = 0;
	int32 LRightSector = 0;
	int32 LSubdivision = 0;
	TArray<int32> PossibleIndex;
	int32 PerimeterCellChosen;
	int32 CurrentCellIndex;
	TArray<FSLabyrinthCell> PotentialNeighbors;
};
