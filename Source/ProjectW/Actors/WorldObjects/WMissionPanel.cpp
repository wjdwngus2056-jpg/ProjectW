// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WMissionPanel.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "ProjectW/FireStation/Framework/WFireStationGameMode.h"

AWMissionPanel::AWMissionPanel()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetRootComponent(InteractionBox);
	InteractionBox->SetBoxExtent(FVector(50.0f, 50.0f, 100.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

bool AWMissionPanel::CanInteract(AActor* Interactor) const
{
	if (!IsValid(Interactor) || MissionId.IsNone())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World || Interactor->GetWorld() != World || !World->GetAuthGameMode<AWFireStationGameMode>())
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation());
	return DistanceSquared <= FMath::Square(InteractionDistance);
}

bool AWMissionPanel::TryInteract(AActor* Interactor)
{
	// 실행 직전에도 요청자, 거리와 GameMode를 다시 확인한다.
	if (!CanInteract(Interactor))
	{
		return false;
	}

	if (AWFireStationGameMode* GameMode = GetWorld()->GetAuthGameMode<AWFireStationGameMode>())
	{
		// 데이터 행 검증과 선택 상태 변경은 GameMode가 담당한다.
		return GameMode->SelectMission(MissionId);
	}

	return false;
}

FText AWMissionPanel::GetInteractionText(AActor* Interactor) const
{
	return NSLOCTEXT("Interaction", "SelectMission", "미션 선택");
}
