// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RTS_PlayerController.h"
#include "prototype1GameMode.generated.h"

UCLASS(minimalapi)
class Aprototype1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	Aprototype1GameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "RTS Setup")
	TSubclassOf<AUnitController> UnitControllerClass;
};


