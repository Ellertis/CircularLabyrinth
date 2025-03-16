// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "LabyMonCul.generated.h"

//Struct
USTRUCT(BlueprintType)
struct FSLabyrinthCell
{
	GENERATED_BODY()
	// Index of the cell
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = 0;

	// Ring number of the cell
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ring = 0;

	// Sector number of the cell
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Sector = 0;

	// Location of the cell in 3D space
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector(0,0,0);

	// Neighbors of the cell (indices of neighboring cells)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Neighbors = TArray<int32>();

	// Whether this cell is the current cell in the pathfinding process
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCurrent = false;

	// Whether this cell has been visited during the pathfinding process
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisited = false;
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

	UPROPERTY(VisibleAnywhere)
	bool RecursiveBacktrackingFinished = false;

	UPROPERTY(VisibleAnywhere)
	bool DebugIndex = false;

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
};
