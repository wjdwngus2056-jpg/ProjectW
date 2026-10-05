// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

AWPlayerController::AWPlayerController()
	: MainMappingContext(nullptr),
	  MoveAction(nullptr)
{
	bShowMouseCursor = true;
}

void AWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* LocalSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (MainMappingContext)
		{
			LocalSubsystem->AddMappingContext(MainMappingContext, 0);
		}
	}
}

void AWPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookCursor();
}

void AWPlayerController::LookCursor()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	APawn* PlayerPawn = GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	FVector Origin;
	FVector Direction;

	if (!DeprojectMousePositionToWorld(Origin, Direction))
	{
		return;
	}

	const FVector PawnLocation = PlayerPawn->GetActorLocation();

	// 캐릭터 발밑 높이를 지나는 수평 조준면
	const FPlane AimPlane(PawnLocation, FVector::UpVector);

	const FVector AimPoint = FMath::LinePlaneIntersection(
		Origin,
		Origin + Direction * 100000.0f,
		AimPlane
	);
	
	FVector AimDirection = AimPoint - PawnLocation;
	AimDirection.Z = 0.0f;
	
	constexpr float AimDeadZone = 30.0f;
	
	if (AimDirection.SizeSquared2D() <= FMath::Square(AimDeadZone))
	{
		return;
	}
	
	const float TargetYaw = AimDirection.Rotation().Yaw;
	SetControlRotation(FRotator(0.0f, TargetYaw, 0.0f));
}
