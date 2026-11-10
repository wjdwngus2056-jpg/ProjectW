// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectW/Actors/Player/WPlayerCharacter.h"

UWPlayerAnimInstance::UWPlayerAnimInstance()
{
}

void UWPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	Owner = Cast<AWPlayerCharacter>(GetOwningActor());
	if (Owner)
	{
		CharacterMovement = Owner->GetCharacterMovement();
	}
}

void UWPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (CharacterMovement)
	{
		const FVector Velocity = CharacterMovement->Velocity;
		
		MovementSpeed = Velocity.Size();
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Owner->GetActorRotation());
		
		bIsFalling = CharacterMovement->IsFalling();
		bIsCrouching = Owner->bIsCrouched;
		bIsJumping = Owner->bPressedJump;
	}
}
