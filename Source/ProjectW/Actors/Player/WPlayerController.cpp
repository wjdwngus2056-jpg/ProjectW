// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerController.h"

AWPlayerController::AWPlayerController()
	: MainMappingContext(nullptr),
	  MoveAction(nullptr),
	  LookAction(nullptr)
{
	bShowMouseCursor = true;
}
