// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ProjectW/Mission/Data/WMissionRuntime.h"
#include "WMissionGameState.generated.h"

/** 현재 시도의 조회 원본. 변경과 판정은 미션 GameMode만 수행한다. */
UCLASS()
class PROJECTW_API AWMissionGameState : public AGameState
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure) EWMissionPhase GetPhase() const { return Phase; }
	UFUNCTION(BlueprintPure) FGuid GetAttemptId() const { return AttemptId; }
	UFUNCTION(BlueprintPure) FName GetMissionId() const { return MissionId; }
	UFUNCTION(BlueprintPure) int32 GetCompletedObjectives() const { return CompletedObjectives; }
	UFUNCTION(BlueprintPure) bool HasResult() const { return bHasResult; }
	UFUNCTION(BlueprintPure) FWMissionResult GetResult() const { return Result; }
	UFUNCTION(BlueprintPure) FText GetTravelError() const { return TravelError; }
private:
	friend class AWMissionGameMode;
	UPROPERTY(VisibleInstanceOnly) EWMissionPhase Phase = EWMissionPhase::Preparing;
	UPROPERTY(VisibleInstanceOnly) FGuid AttemptId;
	UPROPERTY(VisibleInstanceOnly) FName MissionId;
	UPROPERTY(VisibleInstanceOnly) int32 CompletedObjectives = 0;
	UPROPERTY(VisibleInstanceOnly) bool bHasResult = false;
	UPROPERTY(VisibleInstanceOnly) FWMissionResult Result;
	UPROPERTY(VisibleInstanceOnly) FText TravelError;
};
