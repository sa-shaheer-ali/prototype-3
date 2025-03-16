// Copyright Epic Games, Inc. All Rights Reserved.

#include "prototype1Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationSystem.h"


#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// Aprototype1Character

Aprototype1Character::Aprototype1Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->bUseControllerDesiredRotation = true; // Make character face movement direction
	GetCharacterMovement()->bOrientRotationToMovement = true; // Orient character to movement direction

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create first person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FName("head")); // Attach to the head socket of the mesh
	FirstPersonCamera->bUsePawnControlRotation = true; // Camera rotates with the controller
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // Position at the head
	FirstPersonCamera->SetActive(false); // Start with FPS camera disabled

	bIsMoving = false;
	bIsFirstPerson = false; // Start in third person view
	SpawnRadius = 500.0f; // Set spawn radius for FPS view restriction

	// Flow field initialization
	FlowFieldSystem = nullptr;
	FlowFieldUpdateInterval = 0.5f;
	FlowFieldInfluenceRadius = 100.0f;
	TimeSinceLastFlowFieldUpdate = 0.0f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}


void Aprototype1Character::BeginPlay()
{
    Super::BeginPlay();

    // Store the initial spawn location
    SpawnLocation = GetActorLocation();

    // Initialize flow field
    InitializeFlowField();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
        PC->SetInputMode(FInputModeGameAndUI()); // Allows both game and UI interaction
    }
}


//////////////////////////////////////////////////////////////////////////
// Input

void Aprototype1Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void Aprototype1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Click to move (only in RTS view)
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &Aprototype1Character::Click);

		// WASD movement (only in FPS view)
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Aprototype1Character::Move);

		// Mouse look (only in FPS view)
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &Aprototype1Character::Look);

		// Camera switching
		EnhancedInputComponent->BindAction(CameraSwitchAction, ETriggerEvent::Triggered, this, &Aprototype1Character::SwitchCamera);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void Aprototype1Character::Move(const FInputActionValue& Value)
{
    // Only handle movement in FPS view
    if (!bIsFirstPerson)
        return;

    // Get the input vector
    FVector2D MovementVector = Value.Get<FVector2D>();
    
    // Get the forward and right vectors
    FVector ForwardVector = GetActorForwardVector();
    FVector RightVector = GetActorRightVector();
    
    // Calculate movement direction
    FVector MovementDirection = ForwardVector * MovementVector.Y + RightVector * MovementVector.X;
    MovementDirection.Normalize();
    
    // Apply movement
    AddMovementInput(MovementDirection, 1.0f);
}

void Aprototype1Character::Click(const FInputActionValue& Value)
{
    // Only handle clicks in RTS view
    if (bIsFirstPerson)
        return;

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        // Get mouse position in the world
        FVector WorldLocation, WorldDirection;
        PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

        // Perform a raycast to find the clicked location
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this); // Ignore the character itself

        if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, WorldLocation + WorldDirection * 10000, ECC_Visibility, CollisionParams))
        {
            // Move to the clicked location
            MoveToLocation(HitResult.Location);
        }
    }
}

void Aprototype1Character::Look(const FInputActionValue& Value)
{
    // Only handle look input in FPS view
    if (!bIsFirstPerson)
        return;

    // Get the input vector
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    // Add yaw and pitch input to the controller
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        PlayerController->AddYawInput(LookAxisVector.X);
        PlayerController->AddPitchInput(LookAxisVector.Y);
    }
}

void Aprototype1Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMoving && !bIsFirstPerson) // Only update flow field in RTS view
    {
        // Update flow field periodically
        TimeSinceLastFlowFieldUpdate += DeltaTime;
        if (TimeSinceLastFlowFieldUpdate >= FlowFieldUpdateInterval)
        {
            UpdateFlowField();
            TimeSinceLastFlowFieldUpdate = 0.0f;
        }

        // Follow the flow field
        FollowFlowField(DeltaTime);

        // Stop moving if close to the target
        float DistanceToTarget = FVector::Distance(GetActorLocation(), TargetLocation);
        if (DistanceToTarget < 100.0f)
        {
            bIsMoving = false;
            // Stop movement when reaching target
            if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
            {
                MovementComp->Velocity = FVector::ZeroVector;
            }
        }
    }
}

void Aprototype1Character::SwitchCamera()
{
	// If trying to switch to FPS view
	if (!bIsFirstPerson)
	{
		// Check if player is near spawn
		float DistanceFromSpawn = FVector::Distance(GetActorLocation(), SpawnLocation);
		if (DistanceFromSpawn <= SpawnRadius)
		{
			// Allow switching to FPS view
			bIsFirstPerson = true;
			FirstPersonCamera->SetActive(true);
			FollowCamera->SetActive(false);
			CameraBoom->SetActive(false);

			// Configure character for FPS movement
			GetCharacterMovement()->bOrientRotationToMovement = false;
			bUseControllerRotationYaw = true;
			bUseControllerRotationPitch = true;
			bUseControllerRotationRoll = false;

			// Configure mouse cursor for FPS view
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
		else
		{
			// Show a message to the player
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->ClientMessage(TEXT("You can only switch to FPS view near the spawn point!"));
			}
		}
	}
	else
	{
		// Always allow switching back to RTS view
		bIsFirstPerson = false;
		FirstPersonCamera->SetActive(false);
		FollowCamera->SetActive(true);
		CameraBoom->SetActive(true);

		// Configure character for RTS movement
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;
		bUseControllerRotationRoll = false;

		// Configure mouse cursor for RTS view
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}
}

void Aprototype1Character::InitializeFlowField()
{
    // Spawn the flow field system if it doesn't exist
    if (!FlowFieldSystem)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        FlowFieldSystem = GetWorld()->SpawnActor<AFlowFieldSystem>(AFlowFieldSystem::StaticClass(), GetActorLocation(), GetActorRotation(), SpawnParams);
        
        if (FlowFieldSystem)
        {
            // Initialize with a larger world size
            FlowFieldSystem->InitializeFlowField(FVector(5000.0f, 5000.0f, 0.0f), 100.0f);
        }
    }
}

void Aprototype1Character::UpdateFlowField()
{
    if (FlowFieldSystem)
    {
        FlowFieldSystem->UpdateFlowField(TargetLocation);
    }
}

void Aprototype1Character::FollowFlowField(float DeltaTime)
{
    if (!FlowFieldSystem)
        return;

    // Get the flow direction at the current location
    FVector FlowDirection = FlowFieldSystem->GetFlowDirection(GetActorLocation());
    
    // Apply movement in the flow direction
    if (!FlowDirection.IsZero())
    {
        // Get the character's movement component
        UCharacterMovementComponent* MovementComp = GetCharacterMovement();
        if (MovementComp)
        {
            // Use AddMovementInput to properly trigger animations
            AddMovementInput(FlowDirection, 1.0f);
            
            // Make the character face the movement direction
            if (!FlowDirection.IsZero())
            {
                FRotator TargetRotation = FlowDirection.Rotation();
                SetActorRotation(TargetRotation);
            }
        }

        // Debug visualization
        DrawDebugDirectionalArrow(
            GetWorld(),
            GetActorLocation(),
            GetActorLocation() + FlowDirection * 200.0f,
            20.0f,
            FColor::Yellow,
            false,
            -1.0f,
            0,
            2.0f
        );
    }
}

void Aprototype1Character::MoveToLocation(const FVector& Destination)
{
    if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        // Find a path to the destination
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(Destination, NavLocation))
        {
            TargetLocation = NavLocation.Location;
            bIsMoving = true;

            // Update flow field immediately when setting new target
            UpdateFlowField();

            // Draw debug sphere at the destination
            DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Green, false, 2.0f);
            
            // Draw a line from current position to target
            DrawDebugLine(
                GetWorld(),
                GetActorLocation(),
                TargetLocation,
                FColor::Yellow,
                false,
                2.0f,
                0,
                2.0f
            );
        }
    }
}
