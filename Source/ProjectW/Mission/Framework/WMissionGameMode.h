// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Engine/EngineBaseTypes.h"
#include "WMissionGameMode.generated.h"

class UDataTable;
class UWMissionObjectiveComponent;

/**
 * 미션 초기화·목표 보고 검증·결과 확정·복귀를 담당한다. 상태 원본은 미션 GameState다.
 * 목표 보고는 진행만 집계한다. 테스트용 결과 확정 요청과 실제 복귀 요청은 각각 분리한다.
 */
UCLASS()
class PROJECTW_API AWMissionGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AWMissionGameMode();
	/** 이동 옵션을 수신하고 실행 가능한 상태·화면 설정을 준비한다. 유효한 HUD 설정은 보존한다. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	/** 정의·맵·배치를 검증한 뒤 시작한다. 실패 결과에서도 복귀 UI를 사용할 수 있다. */
	virtual void StartPlay() override;
	/** 현재 시도의 등록 대상·ID·중복·진행 상태를 검사한다. 고유 행동 조건은 대상 책임이다. */
	bool CanAcceptObjectiveReport(const UWMissionObjectiveComponent* Source, const FGuid& AttemptId) const;
	/** 수락한 보고만 집계하며 결과 확정·탈출·맵 이동을 호출하지 않는다. */
	bool TryReportObjectiveCompleted(UWMissionObjectiveComponent* Source, const FGuid& AttemptId);
	/** 테스트용: 필수 목표가 끝난 현재 시도의 결과를 별도 요청으로 확정할 수 있는지 조회한다. */
	UFUNCTION(BlueprintPure, Category = "Mission|Development")
	bool CanFinishDevelopmentMission() const;
	/** 테스트용 왕복 검증 경로. 실제 탈출 수락은 전체 루프 연결 작업에서 구현한다. Shipping은 거부한다. */
	UFUNCTION(BlueprintCallable, Category = "Mission|Development")
	bool TryFinishDevelopmentMission(APlayerController* Requester);
	/** 교체 HUD도 사용하는 조회 계약. 결과가 확정되고 아직 복귀 중이 아닐 때만 허용한다. */
	UFUNCTION(BlueprintPure, Category = "Mission")
	bool CanReturnToStation() const;
	/** 로컬 요청자와 결과를 재검증해 이동을 한 번 요청한다. true는 이동 완료가 아닌 수락이다. */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool TryReturnToStation(APlayerController* Requester);
protected:
	/** 월드가 제거되면 이동 실패 구독을 해제한다. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UPROPERTY(EditDefaultsOnly, Category = "Mission") TObjectPtr<UDataTable> MissionDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "Mission") TSoftObjectPtr<UWorld> FireStationLevel;
private:
	/** 새 시도의 소유 상태와 정의·배치 검증. 오류는 진행 대신 복귀 가능한 실패 결과로 연결한다. */
	void InitializeMission();
	/** 초기화 실패 또는 진행 중 종료를 한 번만 확정한다. 맵 이동은 별도 요청에서 처리한다. */
	bool FinalizeResult(bool bSucceeded, const FText& Message);
	/** 결과 이후 플레이를 중단하되 HUD의 복귀 요청 경로는 남긴다. */
	void StopMissionGameplay();
	/** 복귀 이동에 실패하면 결과를 유지한 채 재요청을 허용한다. */
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error);
	FString RequestedMissionId;
	bool bInvalidStateClass = false;
	/** 초기화 때 고정한 대상만 현재 시도에 보고할 수 있다. 종료 시 월드와 함께 제거된다. */
	TMap<FName, TWeakObjectPtr<UWMissionObjectiveComponent>> RegisteredObjectives;
	FDelegateHandle TravelFailureHandle;
};
