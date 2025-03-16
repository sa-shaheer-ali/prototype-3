#include "GridCell.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AGridCell::AGridCell()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create static mesh component for visualization
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    RootComponent = StaticMeshComponent;

    // Initialize default values
    CellState = ECellState::Empty;
    bIsBuildable = true;
    bIsWalkable = true;
    CellSize = 100.0f;
}

void AGridCell::InitializeCell(int32 X, int32 Y, float Size)
{
    GridX = X;
    GridY = Y;
    CellSize = Size;

    // Set the actor's location to the grid position
    FVector Location = FVector(X * Size, Y * Size, 0.0f);
    SetActorLocation(Location);
}

bool AGridCell::IsAvailable() const
{
    return CellState == ECellState::Empty && bIsBuildable;
}

FVector AGridCell::GetCenterLocation() const
{
    return GetActorLocation() + FVector(CellSize * 0.5f, CellSize * 0.5f, 0.0f);
}

void AGridCell::SetCellState(ECellState NewState)
{
    CellState = NewState;
    
    // Update visual appearance based on state
    if (UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(StaticMeshComponent->GetMaterial(0)))
    {
        switch (CellState)
        {
            case ECellState::Empty:
                MaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.2f, 0.2f, 0.5f));
                break;
            case ECellState::Occupied:
                MaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.5f, 0.5f, 0.5f, 0.5f));
                break;
            case ECellState::Blocked:
                MaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.8f, 0.2f, 0.2f, 0.5f));
                break;
        }
    }
}

void AGridCell::HighlightCell(bool bHighlight)
{
    if (UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(StaticMeshComponent->GetMaterial(0)))
    {
        MaterialInstance->SetScalarParameterValue(TEXT("Highlight"), bHighlight ? 1.0f : 0.0f);
    }
} 