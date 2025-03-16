#include "Unit.h"
#include "Kismet/GameplayStatics.h"

AUnit::AUnit()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and setup the unit mesh
    UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
    RootComponent = UnitMesh;
    
    // Setup collision for better unit movement
    UnitMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    UnitMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
    UnitMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    UnitMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    UnitMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    
    // Create movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->UpdatedComponent = RootComponent;
    
    // Set movement parameters
    MovementSpeed = 300.0f;
    RotationSpeed = 10.0f;
    AcceptanceRadius = 50.0f;
    AvoidanceRadius = 200.0f;  // Increased for earlier avoidance
    
    MovementComponent->MaxSpeed = MovementSpeed;
    MovementComponent->Acceleration = MovementSpeed * 4;  // Increased for more responsive movement
    MovementComponent->Deceleration = MovementSpeed * 4;
    MovementComponent->bConstrainToPlane = true;
    MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 1));

    // Initialize variables
    bIsMoving = false;
    TargetDestination = FVector::ZeroVector;
    StuckTime = 0.0f;
    LastLocation = FVector::ZeroVector;
}

void AUnit::BeginPlay()
{
    Super::BeginPlay();
    LastLocation = GetActorLocation();

    // Store the default material
    if (UnitMesh && UnitMesh->GetMaterial(0))
    {
        DefaultMaterial = UnitMesh->GetMaterial(0);
    }
}

void AUnit::SetSelected(bool bSelected)
{
    if (!UnitMesh) return;

    if (bSelected)
    {
        if (SelectedMaterial)
        {
            UnitMesh->SetMaterial(0, SelectedMaterial);
        }
    }
    else
    {
        if (DefaultMaterial)
        {
            UnitMesh->SetMaterial(0, DefaultMaterial);
        }
    }
}

void AUnit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMoving)
    {
        UpdateMovement(DeltaTime);
    }
}

void AUnit::SetDestination(const FVector& NewDestination)
{
    TargetDestination = NewDestination;
    bIsMoving = true;
    StuckTime = 0.0f;
}

void AUnit::UpdateMovement(float DeltaTime)
{
    if (HasReachedDestination())
    {
        bIsMoving = false;
        return;
    }

    FVector CurrentLocation = GetActorLocation();
    
    // Check if unit is stuck
    float MovedDistance = FVector::Distance(CurrentLocation, LastLocation);
    if (MovedDistance < 1.0f && bIsMoving)
    {
        StuckTime += DeltaTime;
    }
    else
    {
        StuckTime = 0.0f;
    }
    LastLocation = CurrentLocation;

    // Calculate base direction to target
    FVector DirectionToTarget = (TargetDestination - CurrentLocation).GetSafeNormal();
    
    // Get all nearby units for avoidance
    TArray<AActor*> NearbyUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), NearbyUnits);
    
    FVector AvoidanceVector = FVector::ZeroVector;
    int32 AvoidCount = 0;
    
    // Calculate avoidance vector with improved close-range response
    for (AActor* OtherActor : NearbyUnits)
    {
        if (OtherActor != this)
        {
            FVector OtherLocation = OtherActor->GetActorLocation();
            float Distance = FVector::Distance(CurrentLocation, OtherLocation);
            
            if (Distance < AvoidanceRadius)
            {
                FVector AwayFromOther = (CurrentLocation - OtherLocation);
                float AvoidanceStrength = 1.0f - (Distance / AvoidanceRadius);
                
                // Exponential avoidance strength for closer units
                AvoidanceStrength = FMath::Pow(AvoidanceStrength, 1.5f);
                
                // Extra strength when very close
                if (Distance < AvoidanceRadius * 0.3f)
                {
                    AvoidanceStrength *= 2.0f;
                }
                
                AvoidanceVector += AwayFromOther.GetSafeNormal() * AvoidanceStrength;
                AvoidCount++;
            }
        }
    }
    
    // Calculate final movement direction with improved blending
    FVector FinalDirection = DirectionToTarget;
    if (AvoidCount > 0)
    {
        AvoidanceVector = AvoidanceVector / AvoidCount;
        
        // Stronger avoidance influence when stuck
        float AvoidanceInfluence = FMath::Clamp(0.4f + (StuckTime * 0.2f), 0.4f, 0.8f);
        float TargetInfluence = 1.0f - AvoidanceInfluence;
        
        // Add slight randomization when stuck to break symmetry
        if (StuckTime > 1.0f)
        {
            FVector RandomOffset = FVector(
                FMath::RandRange(-0.2f, 0.2f),
                FMath::RandRange(-0.2f, 0.2f),
                0.0f
            );
            AvoidanceVector += RandomOffset;
        }
        
        FinalDirection = (DirectionToTarget * TargetInfluence + AvoidanceVector * AvoidanceInfluence).GetSafeNormal();
        
        // Add diagonal movement options when stuck
        if (StuckTime > 0.5f)
        {
            FVector RightVector = FVector::CrossProduct(FVector::UpVector, DirectionToTarget);
            FinalDirection += RightVector * FMath::Sin(StuckTime * 2.0f) * 0.3f;
            FinalDirection = FinalDirection.GetSafeNormal();
        }
    }
    
    // Apply movement with increased force when stuck
    float MovementForce = 1.0f + (StuckTime * 0.5f);
    MovementForce = FMath::Clamp(MovementForce, 1.0f, 2.0f);
    AddMovementInput(FinalDirection, MovementForce);
    
    // Rotate towards movement direction
    FRotator TargetRotation = FinalDirection.Rotation();
    FRotator CurrentRotation = GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
    SetActorRotation(NewRotation);
}

bool AUnit::HasReachedDestination() const
{
    return FVector::Distance(GetActorLocation(), TargetDestination) <= AcceptanceRadius;
}