// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "WheeledVehicleMovementComponent4W.h"

static const FName NAME_SteerInput("Steer");
static const FName NAME_ThrottleInput("Throttle");

AVehiclePawn::AVehiclePawn()
{
	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	// Adjust the tire loading
	Vehicle4W->MinNormalizedTireLoad = 0.0f;
	Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
	Vehicle4W->MaxNormalizedTireLoad = 2.0f;
	Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

	//Torque setup
	Vehicle4W->MaxEngineRPM = 5700.0f;
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(57330.0f, 400.0f);

	//SteeringCurve
	Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

	//Drifting
	Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
	Vehicle4W->DifferentialSetup.FrontLeftRightSplit = 0.65;

	//Automatic gearbox
	Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;
	Vehicle4W->TransmissionSetup.GearAutoBoxLatency = 0.15f;
	Vehicle4W->TransmissionSetup.GearSwitchTime = 1.0f;

	//SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 250.0f;
	SpringArm->bUsePawnControlRotation = true;
	
	//Camera Component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->FieldOfView = 90.f;


}

void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAirControl(DeltaTime);
}

void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	//AxisBinding
	PlayerInputComponent->BindAxis(NAME_ThrottleInput, this, &AVehiclePawn::ApplyThrottle);
	PlayerInputComponent->BindAxis(NAME_SteerInput, this, &AVehiclePawn::ApplySteering);
	PlayerInputComponent->BindAxis("LookUp", this, &AVehiclePawn::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AVehiclePawn::Turn);

	//ActionBinding
	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AVehiclePawn::OnHandBrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AVehiclePawn::OnHandBrakeReleased);
}

void AVehiclePawn::ApplyThrottle(float Val)
{
	GetVehicleMovement()->SetThrottleInput(Val);
}

void AVehiclePawn::ApplySteering(float Val)
{
	GetVehicleMovement()->SetSteeringInput(Val);
}

void AVehiclePawn::LookUp(float Val)
{
	if (Val != 0.f)
		AddControllerPitchInput(Val);
}

void AVehiclePawn::Turn(float Val)
{
	if (Val != 0.f)
	{
		AddControllerYawInput(Val);
	}
}

void AVehiclePawn::OnHandBrakePressed()
{
	GetVehicleMovement()->SetHandbrakeInput(true);
}

void AVehiclePawn::OnHandBrakeReleased()
{
	GetVehicleMovement()->SetHandbrakeInput(false);
}

void AVehiclePawn::UpdateAirControl(float Deltatime)
{
	if (UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement()))
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		const FVector TraceStart = GetActorLocation() + FVector(0.f, 0.f, 50.f);
		const FVector TraceEnd = GetActorLocation() - FVector(0.f, 0.f, 200.f);

		FHitResult Hit;
		//FlipityFlippuCheck
		const bool bInAir = !GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		const bool bNotGrounded = FVector::DotProduct(GetActorUpVector(), FVector::UpVector) < 0.1f;

		//Only Allow in air-movement
		if (bInAir || bNotGrounded)
		{
			const float ForwardInput = InputComponent->GetAxisValue(NAME_ThrottleInput);
			const float RightInput = InputComponent->GetAxisValue(NAME_SteerInput);

			//Grounded Roll
			const float AirMovementForcePitch = 3.f;
			const float AirMovementForceRoll = !bInAir && bNotGrounded ? 20.f : 3.f;

			if (UPrimitiveComponent* VehicleMesh = Vehicle4W->UpdatedPrimitive)
			{
				const FVector MovementVector = FVector(RightInput * -AirMovementForceRoll, ForwardInput * AirMovementForcePitch, 0.f) * Deltatime * 200.f;
				const FVector NewAngularMovement = GetActorRotation().RotateVector(MovementVector);

				VehicleMesh->SetPhysicsAngularVelocity(NewAngularMovement, true);
			}
		}
	}
}
