// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

AWPlayerCharacter::AWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	SpringArmComp->SetRelativeRotation(FRotator(-40.0f, 0.0f, 0.0f));
	SpringArmComp->TargetArmLength = 700.0f;
	
	MainCamera = CreateDefaultSubobject<UCameraComponent>(FName("MainCamera"));
	MainCamera->SetupAttachment(SpringArmComp);
	MainCamera->bUsePawnControlRotation = false;
}

void AWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

