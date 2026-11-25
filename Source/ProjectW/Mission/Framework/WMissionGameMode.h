// Copyright (C) 2026 TeamW. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Engine/EngineBaseTypes.h"
#include "WMissionGameMode.generated.h"

class UDataTable;
class AWDevelopmentMissionObjective;

/** 옵션 검증, 목표 보고, 결과 확정과 복귀 요청을 처리한다. */
UCLASS()
class PROJECTW_API AWMissionGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AWMissionGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	bool CanCompleteDevelopmentObjective(const AWDevelopmentMissionObjective* Source, AActor* Interactor) const;
	bool TryCompleteDevelopmentObjective(AWDevelopmentMissionObjective* Source, AActor* Interactor);
	bool CanReturnToStation() const;
	bool TryReturnToStation(APlayerController* Requester);
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UPROPERTY(EditDefaultsOnly, Category = "Mission") TObjectPtr<UDataTable> MissionDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "Mission") TSoftObjectPtr<UWorld> FireStationLevel;
private:
	void InitializeMission();
	bool FinalizeResult(bool bSucceeded, const FText& Message);
	void StopMissionGameplay();
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error);
	FString RequestedMissionId;
	bool bInvalidStateClass = false;
	TWeakObjectPtr<AWDevelopmentMissionObjective> RegisteredObjective;
	FDelegateHandle TravelFailureHandle;
};
