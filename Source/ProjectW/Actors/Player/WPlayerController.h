// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "WPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UWGameplayInputComponent;
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
	TObjectPtr<UInputMappingContext> MainMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
};
