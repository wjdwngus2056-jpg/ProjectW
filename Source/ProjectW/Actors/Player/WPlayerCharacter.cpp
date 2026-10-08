// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectW/Actors/Interfaces/WInteractable.h"
#include "ProjectW/Core/Framework/WPlayerController.h"

AWPlayerCharacter::AWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	SpringArmComp->SetRelativeRotation(FRotator(-50.0f, 0.0f, 0.0f));
	SpringArmComp->TargetArmLength = 700.0f;

	MainCamera = CreateDefaultSubobject<UCameraComponent>(FName("MainCamera"));
	MainCamera->SetupAttachment(SpringArmComp);
	MainCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate =
		FRotator(0.0f, 720.0f, 0.0f);
}

void AWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* IC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AWPlayerController* PC = Cast<AWPlayerController>(GetController()))
		{
			IC->BindAction(
				PC->MoveAction,
				ETriggerEvent::Triggered,
				this,
				&AWPlayerCharacter::OnMoveAction
			);

			IC->BindAction(
				PC->InteractionAction,
				ETriggerEvent::Started,
				this,
				&AWPlayerCharacter::OnInteractionAction
			);
		}
	}
}

AActor* AWPlayerCharacter::FindInteractionTarget() const
{
	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes =
	{
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_Pawn)
	};

	const TArray<AActor*> IgnoreActors = {GetOwner(), const_cast<AWPlayerCharacter*>(this)};
	TArray<AActor*> Candidates;

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		300.0f,
		ObjectTypes,
		AActor::StaticClass(),
		IgnoreActors,
		Candidates
	);

	AActor* ClosestTarget = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();

	for (AActor* Candidate : Candidates)
	{
		if (!IsValid(Candidate))
		{
			continue;
		}

		IWInteractable* Interactable = Cast<IWInteractable>(Candidate);

		if (!Interactable || !Interactable->CanInteract(const_cast<AWPlayerCharacter*>(this)))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			GetActorLocation(),
			Candidate->GetActorLocation()
		);
		
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestTarget = Candidate;
		}
	}
	
	return ClosestTarget;
}

void AWPlayerCharacter::SetMovementReferenceYaw(float NewYaw)
{
	MovementReferenceYaw = NewYaw;
}

void AWPlayerCharacter::OnMoveAction(const FInputActionValue& value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D MoveInput = value.Get<FVector2D>();

	const FVector Direction = FRotator(0.0f, MovementReferenceYaw, 0.0f)
	                          .RotateVector(FVector(MoveInput.X, MoveInput.Y, 0.0f)).GetClampedToMaxSize(1.0f);

	AddMovementInput(Direction);
}

void AWPlayerCharacter::OnInteractionAction(const FInputActionValue& value)
{
	AActor* TargetActor = FindInteractionTarget();
	
	if (!IsValid(TargetActor))
	{
		return;
	}
	
	if (IWInteractable* Interactable = Cast<IWInteractable>(TargetActor))
	{
		const bool bAccepted = Interactable->TryInteract(this);
	}
}
