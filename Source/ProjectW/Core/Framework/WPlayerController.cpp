// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerController.h"

#include "EnhancedInputSubsystems.h"

AWPlayerController::AWPlayerController()
	: MainMappingContext(nullptr),
	  MoveAction(nullptr),
	  LookAction(nullptr)
{
	bShowMouseCursor = true;
}

void AWPlayerController::BeginPlay()
{
	Super::BeginPlay();

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
