#include "FlowFieldSystem.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"

AFlowFieldSystem::AFlowFieldSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default values with larger world size
    WorldSize = FVector(5000.0f, 5000.0f, 0.0f);
    CellSize = 100.0f;
}

void AFlowFieldSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeFlowField(WorldSize, CellSize);
}

void AFlowFieldSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    DrawDebugFlowField();
}

void AFlowFieldSystem::InitializeFlowField(const FVector& InWorldSize, float InCellSize)
{
    WorldSize = InWorldSize;
    CellSize = InCellSize;

    // Calculate grid dimensions
    GridWidth = FMath::CeilToInt(WorldSize.X / CellSize);
    GridHeight = FMath::CeilToInt(WorldSize.Y / CellSize);

    // Initialize the grid
    FlowFieldGrid.SetNum(GridWidth * GridHeight);
}

FVector2D AFlowFieldSystem::WorldToGrid(const FVector& WorldLocation) const
{
    FVector LocalLocation = WorldLocation - GetActorLocation();
    return FVector2D(
        FMath::FloorToInt(LocalLocation.X / CellSize),
        FMath::FloorToInt(LocalLocation.Y / CellSize)
    );
}

FVector AFlowFieldSystem::GridToWorld(const FVector2D& GridLocation) const
{
    return GetActorLocation() + FVector(
        GridLocation.X * CellSize + CellSize * 0.5f,
        GridLocation.Y * CellSize + CellSize * 0.5f,
        0.0f
    );
}

int32 AFlowFieldSystem::GridToIndex(const FVector2D& GridLocation) const
{
    return GridLocation.Y * GridWidth + GridLocation.X;
}

bool AFlowFieldSystem::IsValidGridLocation(const FVector2D& GridLocation) const
{
    return GridLocation.X >= 0 && GridLocation.X < GridWidth &&
           GridLocation.Y >= 0 && GridLocation.Y < GridHeight;
}

void AFlowFieldSystem::UpdateFlowField(const FVector& TargetLocation)
{
    CalculateFlowField(TargetLocation);
}

void AFlowFieldSystem::CalculateFlowField(const FVector& TargetLocation)
{
    // Reset all cells
    for (auto& Cell : FlowFieldGrid)
    {
        Cell.Cost = FLT_MAX;
        Cell.FlowDirection = FVector2D::ZeroVector;
    }

    // Convert target to grid coordinates
    FVector2D TargetGridLocation = WorldToGrid(TargetLocation);
    if (!IsValidGridLocation(TargetGridLocation))
        return;

    // Set target cell cost to 0
    FlowFieldGrid[GridToIndex(TargetGridLocation)].Cost = 0.0f;

    // Propagate costs from target
    PropagateCosts(TargetGridLocation);

    // Calculate flow directions
    CalculateFlowDirections();
}

void AFlowFieldSystem::PropagateCosts(const FVector2D& TargetGridLocation)
{
    TArray<FVector2D> OpenSet;
    OpenSet.Add(TargetGridLocation);

    while (!OpenSet.IsEmpty())
    {
        FVector2D Current = OpenSet.Pop(false);
        TArray<FVector2D> Neighbors = GetNeighbors(Current);

        for (const FVector2D& Neighbor : Neighbors)
        {
            if (!IsValidGridLocation(Neighbor))
                continue;

            float NewCost = FlowFieldGrid[GridToIndex(Current)].Cost + CalculateCost(Current, Neighbor);
            int32 NeighborIndex = GridToIndex(Neighbor);

            if (NewCost < FlowFieldGrid[NeighborIndex].Cost)
            {
                FlowFieldGrid[NeighborIndex].Cost = NewCost;
                OpenSet.Add(Neighbor);
            }
        }
    }
}

void AFlowFieldSystem::CalculateFlowDirections()
{
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            FVector2D Current(X, Y);
            TArray<FVector2D> Neighbors = GetNeighbors(Current);
            FVector2D LowestCostNeighbor = Current;
            float LowestCost = FlowFieldGrid[GridToIndex(Current)].Cost;

            for (const FVector2D& Neighbor : Neighbors)
            {
                if (!IsValidGridLocation(Neighbor))
                    continue;

                float NeighborCost = FlowFieldGrid[GridToIndex(Neighbor)].Cost;
                if (NeighborCost < LowestCost)
                {
                    LowestCost = NeighborCost;
                    LowestCostNeighbor = Neighbor;
                }
            }

            if (LowestCostNeighbor != Current)
            {
                FlowFieldGrid[GridToIndex(Current)].FlowDirection = (LowestCostNeighbor - Current).GetSafeNormal();
            }
        }
    }
}

TArray<FVector2D> AFlowFieldSystem::GetNeighbors(const FVector2D& Location) const
{
    TArray<FVector2D> Neighbors;
    Neighbors.Add(Location + FVector2D(1, 0));  // Right
    Neighbors.Add(Location + FVector2D(-1, 0)); // Left
    Neighbors.Add(Location + FVector2D(0, 1));  // Up
    Neighbors.Add(Location + FVector2D(0, -1)); // Down
    return Neighbors;
}

float AFlowFieldSystem::CalculateCost(const FVector2D& Current, const FVector2D& Target) const
{
    // Basic Euclidean distance
    return FVector2D::Distance(Current, Target);
}

FVector AFlowFieldSystem::GetFlowDirection(const FVector& WorldLocation) const
{
    FVector2D GridLocation = WorldToGrid(WorldLocation);
    if (!IsValidGridLocation(GridLocation))
        return FVector::ZeroVector;

    // Get the flow direction from the grid
    FVector2D FlowDirection2D = FlowFieldGrid[GridToIndex(GridLocation)].FlowDirection;
    
    // If we have no flow direction, calculate direct path to target
    if (FlowDirection2D.IsNearlyZero())
    {
        // Get direct vector to target (this is a fallback if flow field hasn't been calculated)
        FVector DirectPath = (WorldLocation - GetActorLocation()).GetSafeNormal();
        return DirectPath;
    }

    // Convert 2D direction to 3D and ensure it's normalized
    return FVector(FlowDirection2D.X, FlowDirection2D.Y, 0.0f).GetSafeNormal();
}

void AFlowFieldSystem::DrawDebugFlowField() const
{
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            FVector2D GridLocation(X, Y);
            FVector WorldLocation = GridToWorld(GridLocation);
            FVector FlowDirection = FVector(FlowFieldGrid[GridToIndex(GridLocation)].FlowDirection, 0.0f);

            // Draw flow direction in blue
            DrawDebugDirectionalArrow(
                GetWorld(),
                WorldLocation,
                WorldLocation + FlowDirection * CellSize * 0.5f,
                20.0f,
                FColor::Blue,
                false,
                -1.0f,
                0,
                2.0f
            );

            // Draw cell boundaries in white
            DrawDebugBox(
                GetWorld(),
                WorldLocation,
                FVector(CellSize * 0.5f, CellSize * 0.5f, 0.0f),
                FColor::White,
                false,
                -1.0f,
                0,
                1.0f
            );
        }
    }
} 