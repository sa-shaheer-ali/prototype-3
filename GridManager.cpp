#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Set default grid properties
    GridWidth = 20;
    GridHeight = 20;
    CellSize = 100.0f;
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
    CreateGrid(GridWidth, GridHeight, CellSize);
}

void AGridManager::CreateGrid(int32 Width, int32 Height, float InCellSize)
{
    // Clear existing grid if any
    for (AGridCell* Cell : GridCells)
    {
        if (Cell)
        {
            Cell->Destroy();
        }
    }
    GridCells.Empty();

    // Create new grid
    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            if (GridCellClass)
            {
                FVector Location = GridToWorld(X, Y);
                AGridCell* NewCell = GetWorld()->SpawnActor<AGridCell>(GridCellClass, Location, FRotator::ZeroRotator);
                if (NewCell)
                {
                    NewCell->InitializeCell(X, Y, InCellSize);
                    GridCells.Add(NewCell);
                }
            }
        }
    }
}

AGridCell* AGridManager::GetCell(int32 X, int32 Y) const
{
    if (IsValidGridPosition(X, Y))
    {
        int32 Index = Y * GridWidth + X;
        return GridCells[Index];
    }
    return nullptr;
}

bool AGridManager::IsValidGridPosition(int32 X, int32 Y) const
{
    return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
}

FVector2D AGridManager::WorldToGrid(const FVector& WorldLocation) const
{
    FVector LocalLocation = WorldLocation - GetActorLocation();
    return FVector2D(
        FMath::Floor(LocalLocation.X / CellSize),
        FMath::Floor(LocalLocation.Y / CellSize)
    );
}

FVector AGridManager::GridToWorld(int32 X, int32 Y) const
{
    return GetActorLocation() + FVector(X * CellSize, Y * CellSize, 0.0f);
}

bool AGridManager::IsCellAvailable(int32 X, int32 Y) const
{
    if (AGridCell* Cell = GetCell(X, Y))
    {
        return Cell->IsAvailable();
    }
    return false;
}

void AGridManager::SetCellState(int32 X, int32 Y, ECellState NewState)
{
    if (AGridCell* Cell = GetCell(X, Y))
    {
        Cell->SetCellState(NewState);
    }
}

void AGridManager::HighlightCell(int32 X, int32 Y, bool bHighlight)
{
    if (AGridCell* Cell = GetCell(X, Y))
    {
        Cell->HighlightCell(bHighlight);
    }
} 