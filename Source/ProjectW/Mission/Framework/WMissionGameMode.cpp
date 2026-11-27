// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WMissionGameMode.h"

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "ProjectW/AbilitySystem/Components/WSkillComponent.h"
#include "ProjectW/Mission/Objectives/WMissionObjectiveComponent.h"
#include "ProjectW/Mission/Data/WMissionDefinition.h"
#include "ProjectW/Mission/Framework/WMissionGameState.h"
#include "ProjectW/Mission/Travel/WMissionResultTransfer.h"
#include "ProjectW/UI/Mission/WMissionLoopHUD.h"
#include "UObject/ConstructorHelpers.h"

AWMissionGameMode::AWMissionGameMode()
{
	GameStateClass = AWMissionGameState::StaticClass();
	// 테스트용 임시 UI의 기본값이다. BP 등에서 지정한 교체 HUD는 InitGame에서 유지한다.
	HUDClass = AWMissionLoopHUD::StaticClass();
	static ConstructorHelpers::FObjectFinder<UDataTable> Table(TEXT("/Game/Mission/Data/DT_MissionDefinition"));
	MissionDataTable = Table.Object;
	FireStationLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Level/L_Prototype_FireStation.L_Prototype_FireStation")));
}

void AWMissionGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	// 잘못된 상태 클래스도 오류 결과를 담을 원본은 필요하므로 검증 실패를 기록하고 대체한다.
	bInvalidStateClass = !GameStateClass || !GameStateClass->IsChildOf(AWMissionGameState::StaticClass());
	if (bInvalidStateClass) GameStateClass = AWMissionGameState::StaticClass();
	// 미지정·생성 불가일 때만 임시 UI로 복구한다. 유효한 교체 HUD는 같은 도메인 API를 사용한다.
	if (!HUDClass || HUDClass->HasAnyClassFlags(CLASS_Abstract))
	{
		HUDClass = AWMissionLoopHUD::StaticClass();
	}
	RequestedMissionId = UGameplayStatics::ParseOption(Options, TEXT("MissionId"));
}

void AWMissionGameMode::StartPlay()
{
	// 플레이가 열리기 전에 설정을 판정한다. 실패한 시도는 BeginPlay 후에도 입력을 잠근다.
	InitializeMission();
	Super::StartPlay();
	if (GetGameState<AWMissionGameState>()->HasResult()) StopMissionGameplay();
	TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);
}

void AWMissionGameMode::InitializeMission()
{
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	check(State);
	// 같은 월드의 중복 시작 요청이 새 시도 ID나 이전 확정 결과를 덮어쓰지 않게 한다.
	if (State->AttemptId.IsValid()) return;
	State->AttemptId = FGuid::NewGuid();
	if (UGameInstance* Instance = GetGameInstance()) Instance->GetSubsystem<UWMissionResultTransfer>()->Clear();
	// 초기화 실패도 결과 확정 경로를 사용하여 오류 표시와 복귀 요청을 유지한다.
	auto Fail = [this](const TCHAR* Message) { FinalizeResult(false, FText::FromString(Message)); };
	if (bInvalidStateClass) { Fail(TEXT("미션 GameState 설정이 올바르지 않습니다.")); return; }
	// URL에서 넘어온 문자열은 이름으로 등록하기 전에 길이와 문자를 제한한다.
	if (RequestedMissionId.IsEmpty() || RequestedMissionId.Len() > 64)
	{
		Fail(TEXT("MissionId가 없습니다. 소방서에서 미션을 선택해 출동하세요.")); return;
	}
	for (TCHAR Character : RequestedMissionId)
	{
		if (!FChar::IsAlnum(Character) && Character != TEXT('_'))
		{
			Fail(TEXT("MissionId 형식이 올바르지 않습니다.")); return;
		}
	}
	State->MissionId = FName(*RequestedMissionId);
	if (!MissionDataTable || MissionDataTable->GetRowStruct() != FWMissionDefinition::StaticStruct())
	{
		Fail(TEXT("미션 정의 테이블이 올바르지 않습니다.")); return;
	}
	const FWMissionDefinition* Definition = MissionDataTable->FindRow<FWMissionDefinition>(State->MissionId, TEXT("Mission initialization"), false);
	if (!Definition || Definition->MissionLevel.IsNull() || Definition->RequiredObjectiveIds.IsEmpty())
	{
		Fail(TEXT("MissionId 또는 목표 정의를 찾을 수 없습니다.")); return;
	}
	if (UWorld::RemovePIEPrefix(GetWorld()->GetOutermost()->GetName()) != Definition->MissionLevel.ToSoftObjectPath().GetLongPackageName())
	{
		Fail(TEXT("미션 정의의 목적지와 현재 맵이 다릅니다.")); return;
	}
	// 정의와 배치를 먼저 모두 검증한다. 잘못된 설정에서는 일부 대상도 활성화하지 않는다.
	TSet<FName> ExpectedIds;
	for (FName Id : Definition->RequiredObjectiveIds)
	{
		if (Id.IsNone() || ExpectedIds.Contains(Id))
		{
			Fail(TEXT("필수 목표 ID가 비었거나 중복되었습니다.")); return;
		}
		ExpectedIds.Add(Id);
	}
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		TInlineComponentArray<UWMissionObjectiveComponent*> Objectives(*It);
		for (UWMissionObjectiveComponent* Objective : Objectives)
		{
			const FName Id = Objective->GetObjectiveId();
			if (!ExpectedIds.Contains(Id) || RegisteredObjectives.Contains(Id))
			{
				Fail(TEXT("등록할 목표의 ID가 정의와 다르거나 중복되었습니다.")); return;
			}
			RegisteredObjectives.Add(Id, Objective);
		}
	}
	if (RegisteredObjectives.Num() != ExpectedIds.Num())
	{
		Fail(TEXT("필수 목표의 배치가 누락되었습니다.")); return;
	}
	for (const auto& Entry : RegisteredObjectives) Entry.Value->BeginAttempt(State->AttemptId);
	State->RequiredObjectives = ExpectedIds.Num();
	State->Phase = EWMissionPhase::InProgress;
	UE_LOG(LogTemp, Log, TEXT("MissionLoop Started Mission=%s Attempt=%s Progress=0/%d"),
		*State->MissionId.ToString(), *State->AttemptId.ToString(), State->RequiredObjectives);
}

bool AWMissionGameMode::CanAcceptObjectiveReport(const UWMissionObjectiveComponent* Source, const FGuid& AttemptId) const
{
	const AWMissionGameState* State = GetGameState<AWMissionGameState>();
	if (!State || State->Phase != EWMissionPhase::InProgress || !AttemptId.IsValid() || State->AttemptId != AttemptId ||
		!IsValid(Source) || Source->GetWorld() != GetWorld() || !IsValid(Source->GetOwner())) return false;
	// ID 일치만으로는 부족하다. 이 시도에 등록한 컴포넌트 자체와 완료 내역을 함께 확인한다.
	const TWeakObjectPtr<UWMissionObjectiveComponent>* Registered = RegisteredObjectives.Find(Source->GetObjectiveId());
	return Registered && Registered->Get() == Source && !Source->IsCompleted() &&
		!State->CompletedObjectiveIds.Contains(Source->GetObjectiveId());
}

bool AWMissionGameMode::TryReportObjectiveCompleted(UWMissionObjectiveComponent* Source, const FGuid& AttemptId)
{
	if (!CanAcceptObjectiveReport(Source, AttemptId)) return false;
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	State->CompletedObjectiveIds.Add(Source->GetObjectiveId());
	// 완료는 진행만 바꾼다. 다른 필수 목표와 실제 탈출 수락을 여기서 결과 확정과 결합하지 않는다.
	UE_LOG(LogTemp, Log, TEXT("MissionLoop Objective Attempt=%s Objective=%s Progress=%d/%d"),
		*AttemptId.ToString(), *Source->GetObjectiveId().ToString(), State->GetCompletedObjectives(), State->RequiredObjectives);
	return true;
}

bool AWMissionGameMode::CanFinishDevelopmentMission() const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	const AWMissionGameState* State = GetGameState<AWMissionGameState>();
	return State && State->Phase == EWMissionPhase::InProgress && State->AreRequiredObjectivesComplete();
#endif
}

bool AWMissionGameMode::TryFinishDevelopmentMission(APlayerController* Requester)
{
	// 테스트용 결과 확정도 목표 보고와 별도 요청이다. 정식 탈출 조건을 구현한 것으로 취급하지 않는다.
	if (!CanFinishDevelopmentMission() || !IsValid(Requester) || Requester->GetWorld() != GetWorld() ||
		!Requester->IsLocalController() || !IsValid(Requester->GetPawn())) return false;
	return FinalizeResult(true, NSLOCTEXT("Mission", "DevelopmentResult", "[테스트] 필수 목표 확인 후 결과를 확정했습니다. 실제 탈출은 후속 작업입니다."));
}

bool AWMissionGameMode::FinalizeResult(bool bSucceeded, const FText& Message)
{
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	if (!State || State->bHasResult || (State->Phase != EWMissionPhase::Preparing && State->Phase != EWMissionPhase::InProgress)) return false;
	// 먼저 결과를 확정한 뒤 실행을 중단하여 중복 보고나 정리 중 재진입이 결과를 바꾸지 못하게 한다.
	State->Phase = bSucceeded ? EWMissionPhase::Succeeded : EWMissionPhase::Failed;
	State->bHasResult = true;
	State->Result.AttemptId = State->AttemptId;
	State->Result.MissionId = State->MissionId;
	State->Result.bSucceeded = bSucceeded;
	State->Result.CompletedObjectives = State->GetCompletedObjectives();
	State->Result.RequiredObjectives = State->RequiredObjectives;
	State->Result.Message = Message;
	StopMissionGameplay();
	UE_LOG(LogTemp, Log, TEXT("MissionLoop Result Attempt=%s Success=%d Progress=%d/%d Message=%s"),
		*State->AttemptId.ToString(), bSucceeded, State->GetCompletedObjectives(), State->RequiredObjectives, *Message.ToString());
	return true;
}

void AWMissionGameMode::StopMissionGameplay()
{
	// 캐릭터 입력과 스킬만 닫는다. 초기화 실패와 정상 결과 모두 HUD의 복귀 입력은 살아 있어야 한다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* Controller = It->Get();
		if (!Controller) continue;
		if (APawn* Pawn = Controller->GetPawn())
		{
			if (UPawnMovementComponent* Movement = Pawn->GetMovementComponent()) Movement->StopMovementImmediately();
			if (UWSkillComponent* Skills = Pawn->FindComponentByClass<UWSkillComponent>()) Skills->SetSkillsEnabled(false);
			Pawn->DisableInput(Controller);
		}
	}
}

bool AWMissionGameMode::CanReturnToStation() const
{
	const AWMissionGameState* State = GetGameState<AWMissionGameState>();
	return State && State->bHasResult && (State->Phase == EWMissionPhase::Succeeded || State->Phase == EWMissionPhase::Failed);
}

bool AWMissionGameMode::TryReturnToStation(APlayerController* Requester)
{
	if (!CanReturnToStation() || !IsValid(Requester) || Requester->GetWorld() != GetWorld() || !Requester->IsLocalController()) return false;
	// 테스트용 즉시 복귀도 결과 확정과 분리한다. 목적지를 확인하기 전에는 이동 잠금을 걸지 않는다.
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	if (FireStationLevel.IsNull() || !FPackageName::DoesPackageExist(FireStationLevel.ToSoftObjectPath().GetLongPackageName()))
	{
		State->TravelError = NSLOCTEXT("Mission", "MissingStation", "소방서 맵을 찾지 못했습니다. 맵 설정과 쿠킹을 확인하세요.");
		return false;
	}
	if (UGameInstance* Instance = GetGameInstance()) Instance->GetSubsystem<UWMissionResultTransfer>()->Store(State->Result);
	State->TravelError = FText::GetEmpty();
	// 실제 이동 전에 상태를 잠가 같은 프레임의 반복 입력을 거부한다.
	State->Phase = EWMissionPhase::Returning;
	UE_LOG(LogTemp, Log, TEXT("MissionLoop Returning Attempt=%s"), *State->AttemptId.ToString());
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, FireStationLevel, true);
	return true;
}

void AWMissionGameMode::HandleTravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error)
{
	if (World != GetWorld()) return;
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	if (!State || State->Phase != EWMissionPhase::Returning) return;
	// 확정 결과는 바꾸지 않고 이동 잠금과 전달 대기만 복구하여 같은 시도로 복귀를 재시도한다.
	State->Phase = State->Result.bSucceeded ? EWMissionPhase::Succeeded : EWMissionPhase::Failed;
	State->TravelError = FText::FromString(FString::Printf(TEXT("복귀 실패: %s. R로 다시 시도하세요."), *Error));
	if (UGameInstance* Instance = GetGameInstance()) Instance->GetSubsystem<UWMissionResultTransfer>()->Clear();
}

void AWMissionGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 월드별 GameMode 수명을 넘어 이전 시도의 콜백이 남지 않게 한다.
	if (GEngine) GEngine->OnTravelFailure().Remove(TravelFailureHandle);
	Super::EndPlay(EndPlayReason);
}
