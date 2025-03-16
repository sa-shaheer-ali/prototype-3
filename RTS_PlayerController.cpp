#include "RTS_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Canvas.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ARTS_PlayerController::ARTS_PlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    bIsSelecting = false;
    bIsBuildingMode = false;
    PrimaryActorTick.bCanEverTick = true;  // Enable tick
}

void ARTS_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Find or create the UnitController
    TArray<AActor*> FoundControllers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnitController::StaticClass(), FoundControllers);
    
    if (FoundControllers.Num() > 0)
    {
        UnitController = Cast<AUnitController>(FoundControllers[0]);
    }
    else
    {
        // Spawn a UnitController if none exists
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        UnitController = GetWorld()->SpawnActor<AUnitController>(AUnitController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }

    // Setup Enhanced Input
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // Set input mode to game and UI
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
}

void ARTS_PlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSelecting)
    {
        DrawSelectionBox();
    }

    if (bIsBuildingMode && CurrentBuilding)
    {
        UpdateBuildingPreview();
    }
}

void ARTS_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        // Bind mouse buttons for selection and movement
        EnhancedInputComponent->BindAction(LeftMouseAction, ETriggerEvent::Started, this, &ARTS_PlayerController::OnLeftMouseButtonPressed);
        EnhancedInputComponent->BindAction(LeftMouseAction, ETriggerEvent::Completed, this, &ARTS_PlayerController::OnLeftMouseButtonReleased);
        EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Started, this, &ARTS_PlayerController::OnRightMouseButtonPressed);

        // Building controls
        EnhancedInputComponent->BindAction(StartBuildingAction, ETriggerEvent::Started, this, &ARTS_PlayerController::StartBuildingPlacement);
        EnhancedInputComponent->BindAction(CancelBuildingAction, ETriggerEvent::Started, this, &ARTS_PlayerController::CancelBuildingPlacement);
    }
}

void ARTS_PlayerController::OnLeftMouseButtonPressed()
{
    if (bIsBuildingMode)
    {
        TryPlaceBuilding();
        return;
    }

    if (!UnitController)
        return;

    bIsSelecting = true;
    GetMousePosition(SelectionStart.X, SelectionStart.Y);
    UnitController->StartSelection(SelectionStart);
}

void ARTS_PlayerController::OnLeftMouseButtonReleased()
{
    if (bIsBuildingMode)
        return;

    if (!UnitController)
        return;

    bIsSelecting = false;
    UnitController->EndSelection();
}

void ARTS_PlayerController::OnRightMouseButtonPressed()
{
    if (bIsBuildingMode)
    {
        CancelBuildingPlacement();
        return;
    }

    if (!UnitController)
        return;

    // Get world location under mouse cursor
    FVector WorldLocation;
    if (GetMousePositionInWorld(WorldLocation))
    {
        // Move selected units to clicked location
        UnitController->MoveSelectedUnitsTo(WorldLocation);
    }
}

void ARTS_PlayerController::StartBuildingPlacement()
{
    if (!BuildingClass) 
    {
        UE_LOG(LogTemp, Warning, TEXT("No building class set! Please set a building class in the controller blueprint."));
        return;
    }

    if (CurrentBuilding)
    {
        CurrentBuilding->Destroy();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    CurrentBuilding = GetWorld()->SpawnActor<ABuilding>(BuildingClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
    if (CurrentBuilding)
    {
        bIsBuildingMode = true;
        CurrentBuilding->SetPreviewMode(true);
    }
}

void ARTS_PlayerController::UpdateBuildingPreview()
{
    if (!CurrentBuilding) return;

    FVector WorldLocation;
    if (GetMousePositionInWorld(WorldLocation))
    {
        // Snap to grid
        WorldLocation.X = FMath::RoundToFloat(WorldLocation.X / GridSize) * GridSize;
        WorldLocation.Y = FMath::RoundToFloat(WorldLocation.Y / GridSize) * GridSize;

        CurrentBuilding->SetActorLocation(WorldLocation);
        CurrentBuilding->UpdatePlacementValidation(WorldLocation);
    }
}

void ARTS_PlayerController::TryPlaceBuilding()
{
    if (!CurrentBuilding || !bIsBuildingMode) return;

    FVector WorldLocation;
    if (GetMousePositionInWorld(WorldLocation))
    {
        // Snap to grid
        WorldLocation.X = FMath::RoundToFloat(WorldLocation.X / GridSize) * GridSize;
        WorldLocation.Y = FMath::RoundToFloat(WorldLocation.Y / GridSize) * GridSize;

        CurrentBuilding->SetActorLocation(WorldLocation);
        if (CurrentBuilding->CanBePlaced())
        {
            // Finalize building placement
            CurrentBuilding->OnPlaced();
            
            // Start placing another building immediately
            ABuilding* PlacedBuilding = CurrentBuilding; // Store reference to placed building
            CurrentBuilding = nullptr;
            StartBuildingPlacement(); // This creates a new CurrentBuilding
            
            // Enable collision on the placed building after creating new preview
            if (PlacedBuilding)
            {
                PlacedBuilding->BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
}

void ARTS_PlayerController::CancelBuildingPlacement()
{
    if (CurrentBuilding)
    {
        CurrentBuilding->Destroy();
        CurrentBuilding = nullptr;
    }
    bIsBuildingMode = false;
}

void ARTS_PlayerController::DrawSelectionBox()
{
    if (!bIsSelecting)
        return;

    FVector2D CurrentMousePos;
    GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

    // Draw the selection box using debug lines in world space
    FVector WorldStart, WorldStartDir;
    FVector WorldEnd, WorldEndDir;
    
    DeprojectScreenPositionToWorld(SelectionStart.X, SelectionStart.Y, WorldStart, WorldStartDir);
    DeprojectScreenPositionToWorld(CurrentMousePos.X, CurrentMousePos.Y, WorldEnd, WorldEndDir);

    // Perform line traces to find ground points
    FHitResult HitStart, HitEnd;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    GetWorld()->LineTraceSingleByChannel(HitStart, WorldStart, WorldStart + WorldStartDir * 10000.0f, ECC_Visibility, QueryParams);
    GetWorld()->LineTraceSingleByChannel(HitEnd, WorldEnd, WorldEnd + WorldEndDir * 10000.0f, ECC_Visibility, QueryParams);

    if (HitStart.bBlockingHit && HitEnd.bBlockingHit)
    {
        // Draw debug box in world space
        FVector BoxCenter = (HitStart.Location + HitEnd.Location) * 0.5f;
        FVector BoxExtent = (HitEnd.Location - HitStart.Location).GetAbs() * 0.5f;
        BoxExtent.Z = 100.0f;  // Height of selection box

        DrawDebugBox(
            GetWorld(),
            BoxCenter,
            BoxExtent,
            FQuat::Identity,
            FColor::Green,
            false,
            -1.0f,
            0,
            2.0f
        );
    }
}

bool ARTS_PlayerController::GetMousePositionInWorld(FVector& OutLocation) const
{
    FVector WorldLocation, WorldDirection;
    if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
    {
        // Perform line trace to find ground
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = false;

        if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, WorldLocation + WorldDirection * 10000.0f, ECC_Visibility, QueryParams))
        {
            OutLocation = HitResult.Location;
            return true;
        }
    }
    return false;
}