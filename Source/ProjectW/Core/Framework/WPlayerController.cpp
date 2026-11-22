// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerController.h"

#include "EnhancedInputSubsystems.h"

AWPlayerController::AWPlayerController()
	: MainMappingContext(nullptr),
	  MoveAction(nullptr),
	  InteractionAction(nullptr)
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
	FVector AimDirection;
	if (TryGetCursorAimDirection(AimDirection))
	{
		SetControlRotation(AimDirection.Rotation());
	}
}

bool AWPlayerController::TryGetCursorAimDirection(FVector& OutAimDirection) const
{
	const APawn* PlayerPawn = GetPawn();
	if (!IsLocalPlayerController() || !PlayerPawn)
	{
		return false;
	}

	FVector Origin;
	FVector Direction;
	if (!DeprojectMousePositionToWorld(Origin, Direction) || FMath::IsNearlyZero(Direction.Z))
	{
		return false;
	}

	// 카메라 광선이 캐릭터 높이의 수평면과 만나는 지점을 구합니다.
	const FVector PawnLocation = PlayerPawn->GetActorLocation();
	const double AimDistance = (PawnLocation.Z - Origin.Z) / Direction.Z;
	if (!FMath::IsFinite(AimDistance) || AimDistance <= 0.0)
	{
		return false;
	}

	const FVector AimOffset = Origin + Direction * AimDistance - PawnLocation;
	constexpr float AimDeadZone = 30.0f;
	if (AimOffset.ContainsNaN() || AimOffset.SizeSquared2D() <= FMath::Square(AimDeadZone))
	{
		return false;
	}

	OutAimDirection = AimOffset.GetSafeNormal2D();
	return true;
}
