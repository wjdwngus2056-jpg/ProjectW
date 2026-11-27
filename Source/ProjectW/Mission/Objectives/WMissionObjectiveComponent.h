// Copyright (C) 2026 TeamW. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WMissionObjectiveComponent.generated.h"

/** 소유 Actor의 목표 식별·시도 연결·완료 상태. 구조 등 고유 실행 조건은 소유 Actor가 검증한다. */
UCLASS(ClassGroup = (Mission), meta = (BlueprintSpawnableComponent))
class PROJECTW_API UWMissionObjectiveComponent : public UActorComponent
{

	GENERATED_BODY()
public:
	UWMissionObjectiveComponent();
	/** 정의값이며 실행 상태와 별개다. 등록 후 ID를 바꾸면 기존 등록과 불일치하여 보고가 거부된다. */
	UPROPERTY(EditAnywhere, Category = "Mission") FName ObjectiveId = NAME_None;
	UFUNCTION(BlueprintPure, Category = "Mission") FName GetObjectiveId() const { return ObjectiveId; }
	UFUNCTION(BlueprintPure, Category = "Mission") bool IsCompleted() const { return bCompleted; }
	/** 현재 시도에서 아직 수락하지 않은 등록 대상인지 조회한다. */
	bool CanReportCompletion() const;
	/** 고유 동작을 검증한 소유자가 호출한다. 미션이 보고를 수락한 경우에만 한 번 완료한다. */
	bool TryComplete();
private:
	friend class AWMissionGameMode;
	/** GameMode 등록 시에만 새 시도로 연결한다. 이전 시도의 상태는 전달하지 않는다. */
	void BeginAttempt(const FGuid& InAttemptId);
	UPROPERTY(VisibleInstanceOnly, Category = "Mission") bool bCompleted = false;
	FGuid AttemptId;
};
