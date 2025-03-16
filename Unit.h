#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Unit.generated.h"

UCLASS()
class PROTOTYPE1_API AUnit : public APawn
{
    GENERATED_BODY()

public:
    AUnit();
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    // Movement functions
    void SetDestination(const FVector& NewDestination);
    bool HasReachedDestination() const;

    // Selection functions
    UFUNCTION(BlueprintCallable, Category = "Selection")
    void SetSelected(bool bSelected);

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* UnitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFloatingPawnMovement* MovementComponent;

    // Selection properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selection")
    UMaterialInterface* DefaultMaterial;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selection")
    UMaterialInterface* SelectedMaterial;

    // Movement properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AcceptanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", Meta = (ToolTip = "Radius at which units start avoiding each other"))
    float AvoidanceRadius;

    // Movement state
    bool bIsMoving;
    FVector TargetDestination;
    
    // Stuck detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", Meta = (ToolTip = "Time in seconds that a unit is considered stuck"))
    float StuckTime;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", Meta = (ToolTip = "Last location of the unit"))
    FVector LastLocation;

    // Movement update function
    void UpdateMovement(float DeltaTime);
};