// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "WMissionRuntime.generated.h"

UENUM(BlueprintType)
enum class EWMissionPhase : uint8
{
	Preparing, InProgress, Succeeded, Failed, Returning
};

/** 월드 참조가 없는 확정 결과. 보상과 영구 진행도는 후속 범위다. */
USTRUCT(BlueprintType)
struct PROJECTW_API FWMissionResult
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FGuid AttemptId;
	UPROPERTY(BlueprintReadOnly) FName MissionId;
	UPROPERTY(BlueprintReadOnly) bool bSucceeded = false;
	UPROPERTY(BlueprintReadOnly) int32 CompletedObjectives = 0;
	UPROPERTY(BlueprintReadOnly) FText Message;
};
