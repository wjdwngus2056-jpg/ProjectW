// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WFireStationGameMode.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectW/Mission/Data/WMissionDefinition.h"

bool AWFireStationGameMode::SelectMission(FName MissionId)
{
	if (bMissionTravelRequested || MissionId.IsNone() || !IsValid(MissionDataTable))
	{
		return false;
	}

	const FWMissionDefinition* Mission = MissionDataTable->FindRow<FWMissionDefinition>(
		MissionId, TEXT("AWFireStationGameMode::SelectMission"), false);
	if (!Mission)
	{
		return false;
	}

	SelectedMissionId = MissionId;
	UE_LOG(LogTemp, Log, TEXT("미션 선택: %s"), *SelectedMissionId.ToString());

	return true;
}

bool AWFireStationGameMode::CanStartSelectedMission() const
{
	const FWMissionDefinition* Mission = GetSelectedMissionDefinition();
	return !bMissionTravelRequested && Mission && !Mission->MissionLevel.IsNull();
}

bool AWFireStationGameMode::TryStartSelectedMission()
{
	const FWMissionDefinition* Mission = GetSelectedMissionDefinition();
	if (bMissionTravelRequested || !Mission || Mission->MissionLevel.IsNull())
	{
		return false;
	}

	// 이동 후 생성되는 GameMode에도 선택한 미션을 전달한다.
	const FString Options = FString::Printf(TEXT("MissionId=%s"), *SelectedMissionId.ToString());
	bMissionTravelRequested = true;
	UE_LOG(LogTemp, Log, TEXT("미션 출동 요청: %s -> %s"),
		*SelectedMissionId.ToString(), *Mission->MissionLevel.ToSoftObjectPath().ToString());

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Mission->MissionLevel, true, Options);
	return true;
}

const FWMissionDefinition* AWFireStationGameMode::GetSelectedMissionDefinition() const
{
	if (SelectedMissionId.IsNone() || !IsValid(MissionDataTable))
	{
		return nullptr;
	}

	return MissionDataTable->FindRow<FWMissionDefinition>(
		SelectedMissionId, TEXT("AWFireStationGameMode::GetSelectedMissionDefinition"), false);
}
