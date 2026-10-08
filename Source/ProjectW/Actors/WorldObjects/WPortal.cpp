// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPortal.h"


AWPortal::AWPortal()
{
	PrimaryActorTick.bCanEverTick = true;
}

bool AWPortal::CanInteract(AActor* Interactor) const
{
	if (!IsValid(Interactor) || bHasInteracted)
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(
		GetActorLocation(),
		Interactor->GetActorLocation()
	);
	
	return DistanceSquared <= FMath::Square(InteractionDistance);
}

bool AWPortal::TryInteract(AActor* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return false;
	}
	
	bHasInteracted = true;
	
	UE_LOG(LogTemp, Log, TEXT("%s 포탈 작동 시작"), *GetName());
	
	return true;
}

FText AWPortal::GetInteractionText(AActor* Interactor) const
{
	return NSLOCTEXT("Interaction", "Activate", "포탈작동");
}

void AWPortal::BeginPlay()
{
	Super::BeginPlay();
}

void AWPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
