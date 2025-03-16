#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowFieldSystem.generated.h"

USTRUCT(BlueprintType)
struct FFlowFieldCell
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Flow Field")
    FVector2D FlowDirection;

    UPROPERTY(BlueprintReadWrite, Category = "Flow Field")
    float Cost;

    FFlowFieldCell()
        : FlowDirection(FVector2D::ZeroVector)
        , Cost(0.0f)
    {}
};

UCLASS()
class PROTOTYPE1_API AFlowFieldSystem : public AActor
{
    GENERATED_BODY()

public:
    AFlowFieldSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Initialize the flow field grid
    void InitializeFlowField(const FVector& WorldSize, float CellSize);

    // Update flow field for a new target location
    void UpdateFlowField(const FVector& TargetLocation);

    // Get flow direction at a world location
    FVector GetFlowDirection(const FVector& WorldLocation) const;

    // Debug visualization
    void DrawDebugFlowField() const;

protected:
    // Grid properties
    UPROPERTY(EditAnywhere, Category = "Flow Field")
    FVector WorldSize;

    UPROPERTY(EditAnywhere, Category = "Flow Field")
    float CellSize;

    // Grid dimensions
    int32 GridWidth;
    int32 GridHeight;

    // The actual flow field data
    TArray<FFlowFieldCell> FlowFieldGrid;

    // Helper functions
    FVector2D WorldToGrid(const FVector& WorldLocation) const;
    FVector GridToWorld(const FVector2D& GridLocation) const;
    int32 GridToIndex(const FVector2D& GridLocation) const;
    bool IsValidGridLocation(const FVector2D& GridLocation) const;

    // Flow field calculation
    void CalculateFlowField(const FVector& TargetLocation);
    void PropagateCosts(const FVector2D& TargetGridLocation);
    void CalculateFlowDirections();

private:
    // Cost calculation helpers
    float CalculateCost(const FVector2D& Current, const FVector2D& Target) const;
    TArray<FVector2D> GetNeighbors(const FVector2D& Location) const;
}; 