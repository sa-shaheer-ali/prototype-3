#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GridCell.generated.h"

UENUM(BlueprintType)
enum class ECellState : uint8
{
    Empty       UMETA(DisplayName = "Empty"),
    Occupied    UMETA(DisplayName = "Occupied"),
    Blocked     UMETA(DisplayName = "Blocked")
};

UCLASS()
class PROTOTYPE1_API AGridCell : public AActor
{
    GENERATED_BODY()

public:
    AGridCell();

    // Cell properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    ECellState CellState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bIsBuildable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bIsWalkable;

    // Grid position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridY;

    // Cell size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float CellSize;

    // Functions
    void InitializeCell(int32 X, int32 Y, float Size);
    bool IsAvailable() const;
    FVector GetCenterLocation() const;
    void SetCellState(ECellState NewState);
    void HighlightCell(bool bHighlight);

protected:
    UPROPERTY(VisibleAnywhere, Category = "Grid")
    UStaticMeshComponent* StaticMeshComponent;
}; 