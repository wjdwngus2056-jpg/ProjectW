// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WAbilityContext.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWSkills, Log, All);

/** 입력 자리와 스킬의 분류 태그는 별개입니다. */
UENUM(BlueprintType)
enum class EWAbilitySlot : uint8
{
	BasicAttack,
	SpecialAttack
};

UENUM(BlueprintType)
enum class EWAbilityState : uint8
{
	Idle,
	Preparing,
	Committed,
	Ending
};

enum class EWAbilityRequestStatus : uint8
{
	Accepted,
	Unavailable,
	EmptySlot,
	InvalidAim,
	InvalidDefinition,
	AlreadyActive,
	Cooldown,
	CustomConditionFailed
};

/** 수락은 실행 시작을 뜻합니다. 즉시 종료/취소 결과는 종료 알림으로 전달합니다. */
struct FWAbilityRequestResult
{
	EWAbilityRequestStatus Status = EWAbilityRequestStatus::Unavailable;
	uint64 ExecutionId = 0;

	bool IsAccepted() const { return Status == EWAbilityRequestStatus::Accepted; }
};

struct FWAbilityContext
{
	uint64 ExecutionId = 0;
	FVector AimDirection = FVector::ZeroVector;
};
