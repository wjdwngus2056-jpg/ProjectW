// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WPlayerController.generated.h"
	
/**
 * 
 */
UCLASS()
class PROJECTW_API AWPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AWPlayerController();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	
};
