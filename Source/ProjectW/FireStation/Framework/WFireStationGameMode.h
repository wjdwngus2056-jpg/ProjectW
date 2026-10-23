// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WFireStationGameMode.generated.h"

class UDataTable;
struct FWMissionDefinition;

/** 거점맵에서 사용하는 게임모드 */
UCLASS()
class PROJECTW_API AWFireStationGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	/** 미션 행이 존재할 때만 선택을 변경한다. 실패하면 기존 선택을 유지한다. */
	bool SelectMission(FName MissionId);

	/** 현재 선택된 미션으로 출동을 요청할 수 있는지 조회한다. */
	bool CanStartSelectedMission() const;

	/** 선택된 미션을 다시 검증하고 맵 이동을 요청한다. true는 이동 요청 수락을 뜻한다. */
	bool TryStartSelectedMission();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TObjectPtr<UDataTable> MissionDataTable = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
	FName SelectedMissionId = NAME_None;

	/** 맵 이동 요청 이후 다른 포탈의 중복 요청과 미션 선택 변경을 막는다. */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
	bool bMissionTravelRequested = false;

private:
	const FWMissionDefinition* GetSelectedMissionDefinition() const;
};
