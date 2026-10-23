// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPortal.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectW/FireStation/Framework/WFireStationGameMode.h"


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

	const UWorld* World = GetWorld();
	if (!World || Interactor->GetWorld() != World)
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(
		GetActorLocation(),
		Interactor->GetActorLocation()
	);
	
	if (DistanceSquared > FMath::Square(InteractionDistance))
	{
		return false;
	}

	if (bUseSelectedMission)
	{
		const AWFireStationGameMode* GameMode = World->GetAuthGameMode<AWFireStationGameMode>();
		return GameMode && GameMode->CanStartSelectedMission();
	}

	return !DestinationLevel.IsNull();
}

bool AWPortal::TryInteract(AActor* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return false;
	}
	
	if (bUseSelectedMission)
	{
		AWFireStationGameMode* GameMode = GetWorld()->GetAuthGameMode<AWFireStationGameMode>();
		if (!GameMode || !GameMode->TryStartSelectedMission())
		{
			return false;
		}
	}
	else
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationLevel, true, FString());
	}

	// 수락된 요청만 포탈을 사용한 상태로 기록한다.
	bHasInteracted = true;
	UE_LOG(LogTemp, Log, TEXT("%s 포탈 이동 요청 수락"), *GetName());

	return true;
}

FText AWPortal::GetInteractionText(AActor* Interactor) const
{
	if (bUseSelectedMission)
	{
		return NSLOCTEXT("Interaction", "StartMission", "출동");
	}

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
