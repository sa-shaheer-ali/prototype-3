#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UnitController.h"
#include "Building.h"
#include "InputActionValue.h"
#include "RTS_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class PROTOTYPE1_API ARTS_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ARTS_PlayerController();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

protected:
    virtual void SetupInputComponent() override;

    // Enhanced Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
    UInputAction* LeftMouseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
    UInputAction* RightMouseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
    UInputAction* StartBuildingAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
    UInputAction* CancelBuildingAction;

    // Selection variables
    UPROPERTY()
    bool bIsSelecting;
    UPROPERTY()
    FVector2D SelectionStart;

    // Input functions
    void OnLeftMouseButtonPressed();
    void OnLeftMouseButtonReleased();
    void OnLeftMouseButtonHeld();
    void OnRightMouseButtonPressed();

    // Helper functions
    void DrawSelectionBox();
    bool GetMousePositionInWorld(FVector& OutLocation) const;

    // Building Placement
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RTS|Building")
    TSubclassOf<ABuilding> BuildingClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RTS|Building")
    float GridSize = 100.0f;

    UFUNCTION(BlueprintCallable, Category = "RTS|Building")
    void StartBuildingPlacement();

    UFUNCTION(BlueprintCallable, Category = "RTS|Building")
    void CancelBuildingPlacement();

    UFUNCTION(BlueprintCallable, Category = "RTS|Building")
    void TryPlaceBuilding();

    void UpdateBuildingPreview();

private:
    UPROPERTY()
    class AUnitController* UnitController;

    // Building placement state
    UPROPERTY()
    ABuilding* CurrentBuilding;
    bool bIsBuildingMode;
};