#include "UnitController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AUnitController::AUnitController()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsSelecting = false;
}

void AUnitController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSelecting)
    {
        DrawSelectionBox();
    }
}

AUnit* AUnitController::SpawnUnit(const FVector& SpawnLocation)
{
    if (!UnitClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnitController: UnitClass is not set! Please set it in Blueprint."));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    return GetWorld()->SpawnActor<AUnit>(UnitClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
}

void AUnitController::MoveUnitTo(AUnit* Unit, const FVector& TargetLocation)
{
    if (Unit)
    {
        Unit->SetDestination(TargetLocation);
    }
}

TArray<AUnit*> AUnitController::SpawnUnitsInGrid(const FVector& CenterLocation, int32 Rows, int32 Columns, float Spacing)
{
    TArray<AUnit*> SpawnedUnits;
    
    if (!UnitClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnitController: UnitClass is not set! Please set it in Blueprint."));
        return SpawnedUnits;
    }

    // Calculate start position for grid (top-left corner)
    FVector StartPos = CenterLocation;
    StartPos.X -= (Columns - 1) * Spacing * 0.5f;
    StartPos.Y -= (Rows - 1) * Spacing * 0.5f;

    // Spawn units in grid pattern
    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Column = 0; Column < Columns; Column++)
        {
            FVector SpawnPos = StartPos;
            SpawnPos.X += Column * Spacing;
            SpawnPos.Y += Row * Spacing;

            AUnit* NewUnit = SpawnUnit(SpawnPos);
            if (NewUnit)
            {
                SpawnedUnits.Add(NewUnit);
            }
        }
    }

    return SpawnedUnits;
}

void AUnitController::StartSelection(const FVector2D& ScreenPosition)
{
    bIsSelecting = true;
    SelectionStart = ScreenPosition;
    SelectionEnd = ScreenPosition;
    SelectedUnits.Empty();
}

void AUnitController::UpdateSelection(const FVector2D& CurrentScreenPosition)
{
    if (bIsSelecting)
    {
        SelectionEnd = CurrentScreenPosition;
        UpdateSelectedUnits();
    }
}

void AUnitController::EndSelection()
{
    bIsSelecting = false;
    UpdateSelectedUnits(); // Make sure to update one last time when selection ends
}

void AUnitController::UpdateSelectedUnits()
{
    // First, deselect all previously selected units
    for (AUnit* Unit : SelectedUnits)
    {
        if (Unit)
        {
            Unit->SetSelected(false);
        }
    }
    SelectedUnits.Empty();

    // Get all units in the level
    TArray<AActor*> FoundUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundUnits);

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Get the world bounds of our selection box
    FVector WorldStartPos, WorldStartDir, WorldEndPos, WorldEndDir;
    PC->DeprojectScreenPositionToWorld(SelectionStart.X, SelectionStart.Y, WorldStartPos, WorldStartDir);
    PC->DeprojectScreenPositionToWorld(SelectionEnd.X, SelectionEnd.Y, WorldEndPos, WorldEndDir);

    // Perform line traces to get ground points
    FHitResult HitStart, HitEnd;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    GetWorld()->LineTraceSingleByChannel(HitStart, WorldStartPos, WorldStartPos + WorldStartDir * 10000.0f, ECC_Visibility, QueryParams);
    GetWorld()->LineTraceSingleByChannel(HitEnd, WorldEndPos, WorldEndPos + WorldEndDir * 10000.0f, ECC_Visibility, QueryParams);

    if (!HitStart.bBlockingHit || !HitEnd.bBlockingHit) return;

    // Calculate selection box bounds in world space
    FVector MinBound(
        FMath::Min(HitStart.Location.X, HitEnd.Location.X),
        FMath::Min(HitStart.Location.Y, HitEnd.Location.Y),
        0.0f
    );
    FVector MaxBound(
        FMath::Max(HitStart.Location.X, HitEnd.Location.X),
        FMath::Max(HitStart.Location.Y, HitEnd.Location.Y),
        10000.0f
    );

    // Check each unit if it's in the selection box using world space
    for (AActor* Actor : FoundUnits)
    {
        AUnit* Unit = Cast<AUnit>(Actor);
        if (Unit)
        {
            FVector UnitLocation = Unit->GetActorLocation();
            if (UnitLocation.X >= MinBound.X && UnitLocation.X <= MaxBound.X &&
                UnitLocation.Y >= MinBound.Y && UnitLocation.Y <= MaxBound.Y)
            {
                SelectedUnits.Add(Unit);
                Unit->SetSelected(true);
            }
        }
    }
}

bool AUnitController::IsUnitInSelectionBox(const AUnit* Unit) const
{
    if (!Unit) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return false;

    // Get the world bounds of our selection box
    FVector WorldStartPos, WorldStartDir, WorldEndPos, WorldEndDir;
    PC->DeprojectScreenPositionToWorld(SelectionStart.X, SelectionStart.Y, WorldStartPos, WorldStartDir);
    PC->DeprojectScreenPositionToWorld(SelectionEnd.X, SelectionEnd.Y, WorldEndPos, WorldEndDir);

    // Perform line traces to get ground points
    FHitResult HitStart, HitEnd;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    GetWorld()->LineTraceSingleByChannel(HitStart, WorldStartPos, WorldStartPos + WorldStartDir * 10000.0f, ECC_Visibility, QueryParams);
    GetWorld()->LineTraceSingleByChannel(HitEnd, WorldEndPos, WorldEndPos + WorldEndDir * 10000.0f, ECC_Visibility, QueryParams);

    if (!HitStart.bBlockingHit || !HitEnd.bBlockingHit) return false;

    // Calculate selection box bounds in world space
    FVector MinBound(
        FMath::Min(HitStart.Location.X, HitEnd.Location.X),
        FMath::Min(HitStart.Location.Y, HitEnd.Location.Y),
        0.0f
    );
    FVector MaxBound(
        FMath::Max(HitStart.Location.X, HitEnd.Location.X),
        FMath::Max(HitStart.Location.Y, HitEnd.Location.Y),
        10000.0f
    );

    // Check if unit is within bounds
    FVector UnitLocation = Unit->GetActorLocation();
    return (UnitLocation.X >= MinBound.X && UnitLocation.X <= MaxBound.X &&
            UnitLocation.Y >= MinBound.Y && UnitLocation.Y <= MaxBound.Y);
}

void AUnitController::DrawSelectionBox()
{
    if (!bIsSelecting)
        return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
        return;

    // Convert screen coordinates to world space for the corners
    FVector WorldStart, WorldStartDir;
    FVector WorldEnd, WorldEndDir;
    PC->DeprojectScreenPositionToWorld(SelectionStart.X, SelectionStart.Y, WorldStart, WorldStartDir);
    PC->DeprojectScreenPositionToWorld(SelectionEnd.X, SelectionEnd.Y, WorldEnd, WorldEndDir);

    // Perform line traces to find the ground points
    FHitResult HitStart, HitEnd;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    GetWorld()->LineTraceSingleByChannel(HitStart, WorldStart, WorldStart + WorldStartDir * 10000.0f, ECC_Visibility, QueryParams);
    GetWorld()->LineTraceSingleByChannel(HitEnd, WorldEnd, WorldEnd + WorldEndDir * 10000.0f, ECC_Visibility, QueryParams);

    if (HitStart.bBlockingHit && HitEnd.bBlockingHit)
    {
        // Calculate box center and extent
        FVector Center = (HitStart.Location + HitEnd.Location) * 0.5f;
        FVector Extent = (HitEnd.Location - HitStart.Location).GetAbs() * 0.5f;
        Extent.Z = 100.0f; // Height of the selection box

        // Draw the debug box
        DrawDebugBox(
            GetWorld(),
            Center,
            Extent,
            FQuat::Identity,
            SelectionBoxColor.ToFColor(true),
            false,
            -1.0f,
            0,
            2.0f
        );
    }
}

void AUnitController::MoveSelectedUnitsTo(const FVector& TargetLocation)
{
    for (AUnit* Unit : SelectedUnits)
    {
        if (Unit)
        {
            MoveUnitTo(Unit, TargetLocation);
        }
    }
}