// Copyright Epic Games, Inc. All Rights Reserved.

#include "prototype1GameMode.h"
#include "prototype1Character.h"
#include "UObject/ConstructorHelpers.h"
#include "UnitController.h"
#include "RTS_PlayerController.h"
#include "Kismet/GameplayStatics.h"

Aprototype1GameMode::Aprototype1GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// Set the default player controller to our RTS controller
	PlayerControllerClass = ARTS_PlayerController::StaticClass();

	// Set default unit controller class
	static ConstructorHelpers::FClassFinder<AUnitController> UnitControllerBPClass(TEXT("/Game/Blueprints/BP_UnitController"));
	if (UnitControllerBPClass.Class != nullptr)
	{
		UnitControllerClass = UnitControllerBPClass.Class;
	}
	else
	{
		UnitControllerClass = AUnitController::StaticClass();
	}
}

void Aprototype1GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Always spawn a new UnitController
	if (UnitControllerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// Spawn the controller
		AUnitController* Controller = GetWorld()->SpawnActor<AUnitController>(
			UnitControllerClass, 
			FVector::ZeroVector, 
			FRotator::ZeroRotator, 
			SpawnParams
		);

		// If spawned successfully, create some test units
		if (Controller)
		{
			// Spawn a 3x3 grid of units for testing
			Controller->SpawnUnitsInGrid(FVector(0, 0, 100), 3, 3, 200.0f);
		}
	}
}