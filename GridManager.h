#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.h"
#include "GridManager.generated.h"

UCLASS()
class PROTOTYPE1_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

    virtual void BeginPlay() override;

    // Grid creation and management
    void CreateGrid(int32 Width, int32 Height, float CellSize);
    AGridCell* GetCell(int32 X, int32 Y) const;
    bool IsValidGridPosition(int32 X, int32 Y) const;
    
    // World to grid conversion
    FVector2D WorldToGrid(const FVector& WorldLocation) const;
    FVector GridToWorld(int32 X, int32 Y) const;
    
    // Cell operations
    bool IsCellAvailable(int32 X, int32 Y) const;
    void SetCellState(int32 X, int32 Y, ECellState NewState);
    void HighlightCell(int32 X, int32 Y, bool bHighlight);

    // Grid properties
    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<AGridCell> GridCellClass;

    UPROPERTY(EditAnywhere, Category = "Grid")
    int32 GridWidth;

    UPROPERTY(EditAnywhere, Category = "Grid")
    int32 GridHeight;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float CellSize;

private:
    // Grid data
    UPROPERTY()
    TArray<AGridCell*> GridCells;
}; 