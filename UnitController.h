#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Unit.h"
#include "UnitController.generated.h"

UCLASS()
class PROTOTYPE1_API AUnitController : public AActor
{
    GENERATED_BODY()

public:
    AUnitController();
    virtual void Tick(float DeltaTime) override;

    // Spawn functions
    UFUNCTION(BlueprintCallable, Category = "Unit Control")
    AUnit* SpawnUnit(const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Unit Control")
    TArray<AUnit*> SpawnUnitsInGrid(const FVector& CenterLocation, int32 Rows, int32 Columns, float Spacing);

    // Selection functions
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void StartSelection(const FVector2D& ScreenPosition);

    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void UpdateSelection(const FVector2D& CurrentScreenPosition);

    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void EndSelection();

    // Movement functions
    UFUNCTION(BlueprintCallable, Category = "Unit Control")
    void MoveUnitTo(AUnit* Unit, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Unit Control")
    void MoveSelectedUnitsTo(const FVector& TargetLocation);

    // Selection visualization
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void DrawSelectionBox();

protected:
    // The unit class to spawn
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unit Control")
    TSubclassOf<AUnit> UnitClass;

    UPROPERTY(EditDefaultsOnly, Category = "Unit Control")
    float DefaultSpacing = 200.0f;

    // Selection properties
    UPROPERTY(VisibleAnywhere, Category = "Unit Selection")
    TArray<AUnit*> SelectedUnits;

    UPROPERTY(EditDefaultsOnly, Category = "Unit Selection")
    FLinearColor SelectionBoxColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.3f);

private:
    bool bIsSelecting;
    FVector2D SelectionStart;
    FVector2D SelectionEnd;

    // Helper functions
    void UpdateSelectedUnits();
    bool IsUnitInSelectionBox(const AUnit* Unit) const;
}; 