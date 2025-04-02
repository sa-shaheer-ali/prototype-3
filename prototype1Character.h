// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "FlowFieldSystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"

#include "prototype1Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class Aprototype1Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** First Person Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Click Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClickAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Camera Switch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CameraSwitchAction;

	/** Shoot Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	/** Crosshair Widget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	class UUserWidget* CrosshairWidget;

	/** Shooting range */
	UPROPERTY(EditDefaultsOnly, Category = "FPS")
	float ShootRange = 1000.0f;

public:
	Aprototype1Character();
	
	void Tick(float DeltaTime) override;
	void BeginPlay() override;

protected:
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns FirstPersonCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

protected:
	/** Handle click input */
	void Click(const FInputActionValue& Value);

	/** Handle movement input */
	void Move(const FInputActionValue& Value);

	/** Handle look input */
	void Look(const FInputActionValue& Value);

	/** Move to the clicked location */
	void MoveToLocation(const FVector& Destination);

	/** Switch between first and third person camera */
	void SwitchCamera();

	/** Handle shooting */
	void Shoot();

	/** Show or hide crosshair based on camera mode */
	void UpdateCrosshair();

private:
	// Spawn point restrictions for FPS view
	FVector SpawnLocation;
	float SpawnRadius;

	// Camera state
	bool bIsFirstPerson;

	// Movement state
	bool bIsMoving;
	FVector TargetLocation;

	// Flow field properties
	UPROPERTY()
	AFlowFieldSystem* FlowFieldSystem;
	
	UPROPERTY(EditAnywhere, Category = "Flow Field")
	float FlowFieldUpdateInterval;

	UPROPERTY(EditAnywhere, Category = "Flow Field")
	float FlowFieldInfluenceRadius;

	float TimeSinceLastFlowFieldUpdate;

	// Flow field functions
	void InitializeFlowField();
	void UpdateFlowField();
	void FollowFlowField(float DeltaTime);

	AAIController* AIController; // AI Controller for better pathfinding
	UPathFollowingComponent* PathFollowingComponent; // Component to handle path following
	float AcceptanceRadius; // How close to target before considering it reached
};