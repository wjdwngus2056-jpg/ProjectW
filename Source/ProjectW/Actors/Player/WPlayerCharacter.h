// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WPlayerCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;

/** 플레이어 캐릭터 */
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
	/** 캐릭터의 이동 */
	void OnMoveAction(const FInputActionValue& value);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> MainCamera;
	
public:
	/** 캐릭터 이동 시 기준이 될 Yaw 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementReferenceYaw = 0.0f;
};
