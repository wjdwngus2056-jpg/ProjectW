// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WPlayerCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PROJECTW_API AWPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWPlayerCharacter();
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:
	void SetMovementReferenceYaw(float NewYaw);
	
private:
	void OnMoveAction(const FInputActionValue& value);
	void OnLookAction(const FInputActionValue& value);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> MainCamera;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementReferenceYaw = 0.0f;
};
