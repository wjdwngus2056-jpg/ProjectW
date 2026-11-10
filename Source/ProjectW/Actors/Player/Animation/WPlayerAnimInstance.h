// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WPlayerAnimInstance.generated.h"

class UCharacterMovementComponent;
class AWPlayerCharacter;
/**
 * 
 */
UCLASS()
class PROJECTW_API UWPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UWPlayerAnimInstance();
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimInstance")
	TObjectPtr<AWPlayerCharacter> Owner;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimInstance")
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstance")
	float MovementSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstance")
	float Direction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstance")
	bool bIsFalling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstance")
	bool bIsCrouching;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstance")
	bool bIsJumping;
};
