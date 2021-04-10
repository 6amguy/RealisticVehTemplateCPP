// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "VehiclePawn.generated.h"

/**
 * 
 */
UCLASS()
class CARCPP_API AVehiclePawn : public AWheeledVehicle
{
	GENERATED_BODY()

public:
	AVehiclePawn();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	//Throttle
	void ApplyThrottle(float Val);
	
	//Steering
	void ApplySteering(float Val);

	//Look Around
	void LookUp(float Val);
	void Turn(float Val);

	//Handbrake
	void OnHandBrakePressed();
	void OnHandBrakeReleased();

	//Update air physics
	void UpdateAirControl(float Deltatime);

protected:

	//Spring Arm
	UPROPERTY(Category = Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
		class USpringArmComponent* SpringArm;

	//Camera Component
	UPROPERTY(Category = Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
		class UCameraComponent* Camera;

};
