#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

UENUM(BlueprintType)
enum class EBuildingPlacementState : uint8
{
    Valid,
    InvalidTerrain,
    InvalidOverlap,
    TooFarFromPlayer
};

UCLASS()
class PROTOTYPE1_API ABuilding : public AActor
{
    GENERATED_BODY()

public:
    ABuilding();

    virtual void BeginPlay() override;

    // Building mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    UStaticMeshComponent* BuildingMesh;

    // Materials for different placement states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    UMaterialInterface* PreviewMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    UMaterialInterface* InvalidPlacementMaterial;

    // Placement validation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Placement")
    float MinDistanceToOtherBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Placement")
    TArray<TEnumAsByte<ECollisionChannel>> PlacementSurfaceTypes;

    // Building placement functions
    void SetPreviewMode(bool bEnable);
    void OnPlaced();
    bool CanBePlaced() const;
    void UpdatePlacementValidation(const FVector& Location);
    EBuildingPlacementState ValidatePlacement(const FVector& Location) const;

protected:
    bool bIsPlacementValid;

    // Placement validation checks
    bool CheckSurfaceType(const FVector& Location) const;
    bool CheckBuildingOverlap(const FVector& Location) const;

    // Original material storage
    UPROPERTY()
    UMaterialInterface* OriginalMaterial;
};
