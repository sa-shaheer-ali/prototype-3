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
    
    // Set movement parameters for smoother movement
    MovementSpeed = 400.0f;  // Increased base speed
    RotationSpeed = 8.0f;    // Slightly reduced for smoother rotation
    AcceptanceRadius = 50.0f;
    AvoidanceRadius = 150.0f;  // Reduced to prevent units from spreading too much
    
    MovementComponent->MaxSpeed = MovementSpeed;
    MovementComponent->Acceleration = MovementSpeed * 2;  // Reduced for smoother acceleration
    MovementComponent->Deceleration = MovementSpeed * 2;  // Reduced for smoother deceleration
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
        UE_LOG(LogTemp, Warning, TEXT("%s is moving towards %s"), *GetName(), *TargetDestination.ToString());
        UpdateMovement(DeltaTime);
    }
}

void AUnit::SetDestination(const FVector& NewDestination)
{
    TargetDestination = NewDestination;
    bIsMoving = true;
    StuckTime = 0.0f;

    MovementComponent->Activate(true);  // ✅ Ensure movement is active

    UE_LOG(LogTemp, Warning, TEXT("%s received move command to %s"), *GetName(), *TargetDestination.ToString());
}

void AUnit::UpdateMovement(float DeltaTime)
{
    if (HasReachedDestination())
    {
        bIsMoving = false;
        MovementComponent->StopMovementImmediately();
        return;
    }

    FVector CurrentLocation = GetActorLocation();
    FVector DirectionToTarget = (TargetDestination - CurrentLocation).GetSafeNormal();
    
    // Check if unit is stuck
    float MovedDistance = FVector::Distance(CurrentLocation, LastLocation);
    if (MovedDistance < 1.0f && bIsMoving)
    {
        StuckTime += DeltaTime;
        if (StuckTime > 1.0f)  // If stuck for more than 1 second
        {
            // Add a small random offset to help unstuck
            DirectionToTarget += FVector(FMath::RandRange(-0.3f, 0.3f), FMath::RandRange(-0.3f, 0.3f), 0);
            DirectionToTarget.Normalize();
            StuckTime = 0.0f;  // Reset stuck time
        }
    }
    else
    {
        StuckTime = 0.0f;
    }
    LastLocation = CurrentLocation;

    // Get nearby units for avoidance
    TArray<AActor*> NearbyUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), NearbyUnits);

    FVector AvoidanceVector = FVector::ZeroVector;
    int32 AvoidCount = 0;

    for (AActor* OtherActor : NearbyUnits)
    {
        if (OtherActor != this)
        {
            FVector OtherLocation = OtherActor->GetActorLocation();
            float Distance = FVector::Distance(CurrentLocation, OtherLocation);

            if (Distance < AvoidanceRadius)
            {
                FVector AwayFromOther = (CurrentLocation - OtherLocation).GetSafeNormal();
                float AvoidanceStrength = FMath::Square(1.0f - (Distance / AvoidanceRadius));  // Square for more natural avoidance

                AvoidanceVector += AwayFromOther * AvoidanceStrength;
                AvoidCount++;
            }
        }
    }

    // Blend avoidance with movement direction
    FVector FinalDirection = DirectionToTarget;
    if (AvoidCount > 0)
    {
        AvoidanceVector /= AvoidCount;
        // Adjust the blend factor based on how close we are to other units
        float BlendFactor = FMath::Clamp(AvoidanceVector.Size(), 0.0f, 0.7f);  // Max 70% influence from avoidance
        FinalDirection = FMath::Lerp(DirectionToTarget, AvoidanceVector, BlendFactor).GetSafeNormal();
    }

    // Apply movement
    MovementComponent->AddInputVector(FinalDirection * MovementSpeed * DeltaTime);

    // Smooth rotation
    FRotator TargetRotation = FinalDirection.Rotation();
    FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);
    SetActorRotation(NewRotation);
}

bool AUnit::HasReachedDestination() const
{
    return FVector::Distance(GetActorLocation(), TargetDestination) <= AcceptanceRadius;
}