#include "Building.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ABuilding::ABuilding()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and setup the building mesh
    BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
    RootComponent = BuildingMesh;

    // Setup collision
    BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BuildingMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    BuildingMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Initialize placement parameters
    bIsPlacementValid = false;
    MinDistanceToOtherBuildings = 200.0f;
}

void ABuilding::BeginPlay()
{
    Super::BeginPlay();
    
    // Store the original material
    if (BuildingMesh && BuildingMesh->GetMaterial(0))
    {
        OriginalMaterial = BuildingMesh->GetMaterial(0);
    }
}

void ABuilding::SetPreviewMode(bool bEnable)
{
    if (!BuildingMesh) return;

    if (bEnable)
    {
        if (PreviewMaterial)
        {
            BuildingMesh->SetMaterial(0, PreviewMaterial);
        }
        BuildingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else
    {
        // Restore original material and collision
        if (OriginalMaterial)
        {
            BuildingMesh->SetMaterial(0, OriginalMaterial);
        }
        BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void ABuilding::OnPlaced()
{
    // Restore original material if available
    if (BuildingMesh && OriginalMaterial)
    {
        BuildingMesh->SetMaterial(0, OriginalMaterial);
    }

    // Enable physics and collision
    BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

bool ABuilding::CanBePlaced() const
{
    return bIsPlacementValid;
}

void ABuilding::UpdatePlacementValidation(const FVector& Location)
{
    EBuildingPlacementState PlacementState = ValidatePlacement(Location);
    bIsPlacementValid = (PlacementState == EBuildingPlacementState::Valid);

    // Update visual feedback
    if (BuildingMesh)
    {
        if (bIsPlacementValid && ValidPlacementMaterial)
        {
            BuildingMesh->SetMaterial(0, ValidPlacementMaterial);
        }
        else if (!bIsPlacementValid && InvalidPlacementMaterial)
        {
            BuildingMesh->SetMaterial(0, InvalidPlacementMaterial);
        }
    }
}

EBuildingPlacementState ABuilding::ValidatePlacement(const FVector& Location) const
{
    // Check surface type
    if (!CheckSurfaceType(Location))
    {
        return EBuildingPlacementState::InvalidTerrain;
    }

    // Check for overlapping buildings
    if (!CheckBuildingOverlap(Location))
    {
        return EBuildingPlacementState::InvalidOverlap;
    }

    return EBuildingPlacementState::Valid;
}

bool ABuilding::CheckSurfaceType(const FVector& Location) const
{
    if (PlacementSurfaceTypes.Num() == 0) return true;

    FHitResult HitResult;
    FVector Start = Location + FVector(0, 0, 100);
    FVector End = Location + FVector(0, 0, -100);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        for (auto& SurfaceType : PlacementSurfaceTypes)
        {
            if (HitResult.Component->GetCollisionObjectType() == SurfaceType)
            {
                return true;
            }
        }
    }

    return false;
}

bool ABuilding::CheckBuildingOverlap(const FVector& Location) const
{
    TArray<AActor*> OverlappingBuildings;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), OverlappingBuildings);

    for (const AActor* OtherBuilding : OverlappingBuildings)
    {
        if (OtherBuilding != this)
        {
            float Distance = FVector::Dist(Location, OtherBuilding->GetActorLocation());
            if (Distance < MinDistanceToOtherBuildings)
            {
                return false;
            }
        }
    }

    return true;
}
