// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WFireStationGameMode.h"

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Misc/PackageName.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectW/Mission/Data/WMissionDefinition.h"
#include "ProjectW/Mission/Travel/WMissionResultTransfer.h"
#include "ProjectW/UI/Mission/WMissionLoopHUD.h"

AWFireStationGameMode::AWFireStationGameMode()
{
	HUDClass = AWMissionLoopHUD::StaticClass();
}

void AWFireStationGameMode::BeginPlay()
{
	Super::BeginPlay();
	TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);
	if (UGameInstance* Instance = GetGameInstance()) bHasLastResult = Instance->GetSubsystem<UWMissionResultTransfer>()->Consume(LastResult);
	UE_LOG(LogTemp, Log, TEXT("MissionLoop Station Ready PreviousResult=%d Attempt=%s"), bHasLastResult, *LastResult.AttemptId.ToString());
}

void AWFireStationGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GEngine) GEngine->OnTravelFailure().Remove(TravelFailureHandle);
	Super::EndPlay(EndPlayReason);
}

void AWFireStationGameMode::HandleTravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error)
{
	if (World != GetWorld()) return;
	bMissionTravelRequested = false;
	TravelError = FText::FromString(FString::Printf(TEXT("출동 실패: %s. 다시 시도하세요."), *Error));
}

bool AWFireStationGameMode::SelectMission(FName MissionId)
{
	if (bMissionTravelRequested || MissionId.IsNone() || !IsValid(MissionDataTable) || MissionDataTable->GetRowStruct() != FWMissionDefinition::StaticStruct())
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
	TravelError = FText::GetEmpty();
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
	if (!FPackageName::DoesPackageExist(Mission->MissionLevel.ToSoftObjectPath().GetLongPackageName()))
	{
		TravelError = NSLOCTEXT("Mission", "MissingDestination", "출동할 맵이 없습니다. 맵 설정과 쿠킹을 확인하세요.");
		return false;
	}

	// 이동 후 생성되는 GameMode에도 선택한 미션을 전달한다.
	const FString Options = FString::Printf(TEXT("MissionId=%s"), *SelectedMissionId.ToString());
	bMissionTravelRequested = true;
	TravelError = FText::GetEmpty();
	UE_LOG(LogTemp, Log, TEXT("미션 출동 요청: %s -> %s"),
		*SelectedMissionId.ToString(), *Mission->MissionLevel.ToSoftObjectPath().ToString());

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Mission->MissionLevel, true, Options);
	return true;
}

const FWMissionDefinition* AWFireStationGameMode::GetSelectedMissionDefinition() const
{
	if (SelectedMissionId.IsNone() || !IsValid(MissionDataTable) || MissionDataTable->GetRowStruct() != FWMissionDefinition::StaticStruct())
	{
		return nullptr;
	}

	return MissionDataTable->FindRow<FWMissionDefinition>(
		SelectedMissionId, TEXT("AWFireStationGameMode::GetSelectedMissionDefinition"), false);
}
