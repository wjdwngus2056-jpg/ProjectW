// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPortal.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectW/FireStation/Framework/WFireStationGameMode.h"
#include "ProjectW/Mission/Framework/WMissionGameMode.h"
#include "GameFramework/Pawn.h"


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

	if (const AWMissionGameMode* MissionMode = World->GetAuthGameMode<AWMissionGameMode>())
	{
		return MissionMode->CanReturnToStation();
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
		if (AWMissionGameMode* MissionMode = GetWorld()->GetAuthGameMode<AWMissionGameMode>())
		{
			APawn* Pawn = Cast<APawn>(Interactor);
			return Pawn && MissionMode->TryReturnToStation(Cast<APlayerController>(Pawn->GetController()));
		}
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationLevel, true, FString());
	}

	// 수락된 요청만 포탈을 사용한 상태로 기록한다.
	// 선택 미션 출동은 GameMode가 잠근다. 이동 실패 뒤 포탈도 재사용 가능해야 한다.
	bHasInteracted = !bUseSelectedMission;
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
