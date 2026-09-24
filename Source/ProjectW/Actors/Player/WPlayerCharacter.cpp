// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "WPlayerController.h"
#include "Camera/CameraComponent.h"
#include "DataWrappers/ChaosVDParticleDataWrapper.h"
#include "GameFramework/SpringArmComponent.h"
#include "ProfilingDebugging/SpatialTrace.h"

AWPlayerCharacter::AWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	SpringArmComp->SetRelativeRotation(FRotator(-40.0f, 0.0f, 0.0f));
	SpringArmComp->TargetArmLength = 700.0f;

	MainCamera = CreateDefaultSubobject<UCameraComponent>(FName("MainCamera"));
	MainCamera->SetupAttachment(SpringArmComp);
	MainCamera->bUsePawnControlRotation = false;
}

void AWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* IC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AWPlayerController* PC = Cast<AWPlayerController>(GetController()))
		{
			IC->BindAction(
				PC->MoveAction,
				ETriggerEvent::Triggered,
				this,
				&AWPlayerCharacter::OnMoveAction
			);

			IC->BindAction(
				PC->LookAction,
				ETriggerEvent::Triggered,
				this,
				&AWPlayerCharacter::OnLookAction
			);
		}
	}
}

void AWPlayerCharacter::SetMovementReferenceYaw(float NewYaw)
{
	MovementReferenceYaw = NewYaw;
}

void AWPlayerCharacter::OnMoveAction(const FInputActionValue& value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D MoveInput = value.Get<FVector2D>();

	const FVector Direction = FRotator(0.0f, MovementReferenceYaw, 0.0f)
		.RotateVector(FVector(MoveInput.X, MoveInput.Y, 0.0f)).GetClampedToMaxSize(1.0f);

	AddMovementInput(Direction);
}

void AWPlayerCharacter::OnLookAction(const FInputActionValue& value)
{
}
