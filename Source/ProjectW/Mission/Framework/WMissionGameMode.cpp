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
#include "ProjectW/Actors/WorldObjects/WDevelopmentMissionObjective.h"
#include "ProjectW/Mission/Data/WMissionDefinition.h"
#include "ProjectW/Mission/Framework/WMissionGameState.h"
#include "ProjectW/Mission/Travel/WMissionResultTransfer.h"
#include "ProjectW/UI/Mission/WMissionLoopHUD.h"
#include "UObject/ConstructorHelpers.h"

AWMissionGameMode::AWMissionGameMode()
{
	GameStateClass = AWMissionGameState::StaticClass();
	HUDClass = AWMissionLoopHUD::StaticClass();
	static ConstructorHelpers::FObjectFinder<UDataTable> Table(TEXT("/Game/Mission/Data/DT_MissionDefinition"));
	MissionDataTable = Table.Object;
	FireStationLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Level/L_Prototype_FireStation.L_Prototype_FireStation")));
}

void AWMissionGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	// 잘못된 BP 설정에서도 오류를 조회할 GameState와 복귀 HUD는 제공한다.
	bInvalidStateClass = !GameStateClass || !GameStateClass->IsChildOf(AWMissionGameState::StaticClass());
	if (bInvalidStateClass) GameStateClass = AWMissionGameState::StaticClass();
	HUDClass = AWMissionLoopHUD::StaticClass();
	RequestedMissionId = UGameplayStatics::ParseOption(Options, TEXT("MissionId"));
}

void AWMissionGameMode::StartPlay()
{
	InitializeMission();
	Super::StartPlay();
	if (GetGameState<AWMissionGameState>()->HasResult()) StopMissionGameplay();
	TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);
}

void AWMissionGameMode::InitializeMission()
{
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	check(State);
	if (State->AttemptId.IsValid()) return;
	State->AttemptId = FGuid::NewGuid();
	if (UGameInstance* Instance = GetGameInstance()) Instance->GetSubsystem<UWMissionResultTransfer>()->Clear();
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
	if (!Definition || Definition->MissionLevel.IsNull() || Definition->DevelopmentObjectiveId.IsNone())
	{
		Fail(TEXT("MissionId 또는 목표 정의를 찾을 수 없습니다.")); return;
	}
	if (UWorld::RemovePIEPrefix(GetWorld()->GetOutermost()->GetName()) != Definition->MissionLevel.ToSoftObjectPath().GetLongPackageName())
	{
		Fail(TEXT("미션 정의의 목적지와 현재 맵이 다릅니다.")); return;
	}
#if UE_BUILD_SHIPPING
	Fail(TEXT("개발용 목표는 Shipping에서 사용할 수 없습니다.")); return;
#else
	int32 Count = 0;
	for (TActorIterator<AWDevelopmentMissionObjective> It(GetWorld()); It; ++It)
	{
		++Count;
		RegisteredObjective = *It;
	}
	if (Count != 1 || RegisteredObjective->GetObjectiveId() != Definition->DevelopmentObjectiveId)
	{
		Fail(TEXT("개발용 목표 배치가 잘못되었습니다. 목표 1개와 일치하는 ID가 필요합니다.")); return;
	}
	State->Phase = EWMissionPhase::InProgress;
	UE_LOG(LogTemp, Log, TEXT("MissionLoop Started Mission=%s Attempt=%s Progress=0/1"), *State->MissionId.ToString(), *State->AttemptId.ToString());
#endif
}

bool AWMissionGameMode::CanCompleteDevelopmentObjective(const AWDevelopmentMissionObjective* Source, AActor* Interactor) const
{
	const AWMissionGameState* State = GetGameState<AWMissionGameState>();
	const APawn* Pawn = Cast<APawn>(Interactor);
	return State && State->Phase == EWMissionPhase::InProgress && Source && RegisteredObjective.Get() == Source &&
		Pawn && Pawn->IsPlayerControlled() && Pawn->GetWorld() == GetWorld() &&
		FVector::DistSquared(Source->GetActorLocation(), Pawn->GetActorLocation()) <= FMath::Square(200.0f);
}

bool AWMissionGameMode::TryCompleteDevelopmentObjective(AWDevelopmentMissionObjective* Source, AActor* Interactor)
{
	if (!CanCompleteDevelopmentObjective(Source, Interactor)) return false;
	GetGameState<AWMissionGameState>()->CompletedObjectives = 1;
	return FinalizeResult(true, NSLOCTEXT("Mission", "PrototypeDone", "개발용 목표를 완료했습니다."));
}

bool AWMissionGameMode::FinalizeResult(bool bSucceeded, const FText& Message)
{
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	if (!State || State->bHasResult || (State->Phase != EWMissionPhase::Preparing && State->Phase != EWMissionPhase::InProgress)) return false;
	State->Phase = bSucceeded ? EWMissionPhase::Succeeded : EWMissionPhase::Failed;
	State->bHasResult = true;
	State->Result.AttemptId = State->AttemptId;
	State->Result.MissionId = State->MissionId;
	State->Result.bSucceeded = bSucceeded;
	State->Result.CompletedObjectives = State->CompletedObjectives;
	State->Result.Message = Message;
	StopMissionGameplay();
	UE_LOG(LogTemp, Log, TEXT("MissionLoop Result Attempt=%s Success=%d Progress=%d/1 Message=%s"),
		*State->AttemptId.ToString(), bSucceeded, State->CompletedObjectives, *Message.ToString());
	return true;
}

void AWMissionGameMode::StopMissionGameplay()
{
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
	AWMissionGameState* State = GetGameState<AWMissionGameState>();
	if (FireStationLevel.IsNull() || !FPackageName::DoesPackageExist(FireStationLevel.ToSoftObjectPath().GetLongPackageName()))
	{
		State->TravelError = NSLOCTEXT("Mission", "MissingStation", "소방서 맵을 찾지 못했습니다. 맵 설정과 쿠킹을 확인하세요.");
		return false;
	}
	if (UGameInstance* Instance = GetGameInstance()) Instance->GetSubsystem<UWMissionResultTransfer>()->Store(State->Result);
	State->TravelError = FText::GetEmpty();
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
	State->Phase = State->Result.bSucceeded ? EWMissionPhase::Succeeded : EWMissionPhase::Failed;
	State->TravelError = FText::FromString(FString::Printf(TEXT("복귀 실패: %s. R로 다시 시도하세요."), *Error));
	if (UGameInstance* Instance = GetGameInstance()) Instance->GetSubsystem<UWMissionResultTransfer>()->Clear();
}

void AWMissionGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GEngine) GEngine->OnTravelFailure().Remove(TravelFailureHandle);
	Super::EndPlay(EndPlayReason);
}
