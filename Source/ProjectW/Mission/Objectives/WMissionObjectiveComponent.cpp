// Copyright (C) 2026 TeamW. All Rights Reserved.
#include "WMissionObjectiveComponent.h"
#include "Engine/World.h"
#include "ProjectW/Mission/Framework/WMissionGameMode.h"

UWMissionObjectiveComponent::UWMissionObjectiveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWMissionObjectiveComponent::BeginAttempt(const FGuid& InAttemptId)
{
	// 미션 월드가 등록한 시도에만 보고한다. 에셋에 실행 상태를 저장하지 않는다.
	AttemptId = InAttemptId;
	bCompleted = false;
}

bool UWMissionObjectiveComponent::CanReportCompletion() const
{
	const AWMissionGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<AWMissionGameMode>() : nullptr;
	return !bCompleted && Mode && Mode->CanAcceptObjectiveReport(this, AttemptId);
}

bool UWMissionObjectiveComponent::TryComplete()
{
	AWMissionGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<AWMissionGameMode>() : nullptr;
	// 거부된 보고는 구조 상태도 바꾸지 않는다. 집계 수락 후 소유 대상의 최초 완료를 기록한다.
	if (bCompleted || !Mode || !Mode->TryReportObjectiveCompleted(this, AttemptId)) return false;
	bCompleted = true;
	return true;
}
