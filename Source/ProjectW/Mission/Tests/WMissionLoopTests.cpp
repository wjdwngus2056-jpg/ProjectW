// Copyright (C) 2026 TeamW. All Rights Reserved.
#include "ProjectW/Mission/Framework/WMissionGameMode.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "ProjectW/Actors/WorldObjects/WRescueTarget.h"
#include "ProjectW/Mission/Objectives/WMissionObjectiveComponent.h"
#include "ProjectW/Actors/Player/WPlayerCharacter.h"
#include "ProjectW/AbilitySystem/Components/WSkillComponent.h"
#include "ProjectW/Mission/Data/WMissionDefinition.h"
#include "ProjectW/Mission/Framework/WMissionGameState.h"
#include "ProjectW/Mission/Travel/WMissionResultTransfer.h"
#include "UObject/UnrealType.h"

namespace
{
	struct FMissionTestWorld
	{
		UWorld* World;
		UGameInstance* Instance;
		AWMissionGameMode* Mode;
		AWRescueTarget* Target = nullptr;
		AWPlayerCharacter* Player;
		APlayerController* Controller;
		FMissionTestWorld(const FString& MissionId = TEXT("Mission_Test"), int32 Targets = 1, bool bMissingTable = false, bool bWrongMap = false)
		{
			Instance = NewObject<UGameInstance>(GEngine);
			Instance->InitializeStandalone(FName(TEXT("MissionLoopTest")));
			World = Instance->GetWorld();
			FURL URL(nullptr, *FString::Printf(TEXT("/Game/Level/L_Prototype_Mission?game=/Script/ProjectW.WMissionGameMode?MissionId=%s"), *MissionId), TRAVEL_Absolute);
			World->SetGameMode(URL);
			Mode = World->GetAuthGameMode<AWMissionGameMode>();
			UDataTable* Table = NewObject<UDataTable>(Mode);
			Table->RowStruct = FWMissionDefinition::StaticStruct();
			FWMissionDefinition Definition;
			Definition.MissionLevel = bWrongMap ? TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Level/Wrong.Wrong"))) : TSoftObjectPtr<UWorld>(World);
			Table->AddRow(TEXT("Mission_Test"), Definition);
			FindFProperty<FObjectProperty>(Mode->GetClass(), TEXT("MissionDataTable"))->SetObjectPropertyValue_InContainer(Mode, bMissingTable ? nullptr : Table);
			for (int32 Index = 0; Index < Targets; ++Index) Target = World->SpawnActor<AWRescueTarget>(FVector(100, 0, 100), FRotator::ZeroRotator);
			Controller = World->SpawnActor<APlayerController>();
			Player = World->SpawnActor<AWPlayerCharacter>(FVector(0, 0, 100), FRotator::ZeroRotator);
			World->InitializeActorsForPlay(URL);
			// PlayerState 생성 이후 빙의하여 실제 플레이어 판별 조건을 충족한다.
			Controller->Possess(Player);
			World->BeginPlay();
		}
		~FMissionTestWorld()
		{
			World->EndPlay(EEndPlayReason::Quit);
			Instance->Shutdown();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
		AWMissionGameState* State() const { return World->GetGameState<AWMissionGameState>(); }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMissionResultTest, "ProjectW.MissionLoop.CompletionAndFreshAttempt",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMissionResultTest::RunTest(const FString& Parameters)
{
	FGuid Previous;
	for (int32 Attempt = 0; Attempt < 3; ++Attempt)
	{
		FMissionTestWorld TestWorld;
		AWMissionGameState* State = TestWorld.State();
		if (!TestNotNull(TEXT("Mission GameState"), State)) return false;
		TestTrue(TEXT("Starts in progress"), State->GetPhase() == EWMissionPhase::InProgress);
		TestEqual(TEXT("New attempt has zero progress"), State->GetCompletedObjectives(), 0);
		TestFalse(TEXT("No previous result"), State->HasResult());
		TestTrue(TEXT("Fixture owns player state"), TestWorld.Player->IsPlayerControlled());
		TestTrue(TEXT("Fresh attempt ID"), State->GetAttemptId().IsValid() && State->GetAttemptId() != Previous);
		Previous = State->GetAttemptId();
		TestFalse(TEXT("Cannot return before completion"), TestWorld.Mode->TryReturnToStation(TestWorld.Controller));
		TestFalse(TEXT("Null interactor rejected"), TestWorld.Target->TryInteract(nullptr));
		TestWorld.Player->SetActorLocation(FVector(-1000, 0, 100));
		TestFalse(TEXT("Distant request rejected"), TestWorld.Target->TryInteract(TestWorld.Player));
		TestWorld.Player->SetActorLocation(FVector(0, 0, 100));
		AWRescueTarget* Rogue = TestWorld.World->SpawnActor<AWRescueTarget>();
		TestFalse(TEXT("Unregistered target rejected"), Rogue->TryInteract(TestWorld.Player));
		TestTrue(TEXT("Registered interaction completes"), TestWorld.Target->TryInteract(TestWorld.Player));
		// 시민 구조는 목표만 완료한다. 왕복 결과는 별도 테스트 요청으로 확정한다.
		TestTrue(TEXT("Citizen owns rescued state"), TestWorld.Target->IsRescued());
		TestFalse(TEXT("Rescue alone does not finalize result"), State->HasResult());
		TestTrue(TEXT("Development finish requested separately"), TestWorld.Mode->TryFinishDevelopmentMission(TestWorld.Controller));
		const FWMissionResult Result = State->GetResult();
		TestTrue(TEXT("Success result fixed"), Result.bSucceeded && Result.AttemptId == Previous);
		TestEqual(TEXT("One completed target"), Result.CompletedObjectives, 1);
		for (int32 Duplicate = 0; Duplicate < 5; ++Duplicate) TestFalse(TEXT("Repeated report rejected"), TestWorld.Target->TryInteract(TestWorld.Player));
		TestEqual(TEXT("No duplicate progress"), State->GetCompletedObjectives(), 1);
		TestTrue(TEXT("Skills reject requests at result"), TestWorld.Player->FindComponentByClass<UWSkillComponent>()->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::ForwardVector).Status == EWAbilityRequestStatus::Unavailable);
		TestTrue(TEXT("Return available"), TestWorld.Mode->CanReturnToStation());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMissionInitializationTest, "ProjectW.MissionLoop.InitializationErrors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMissionInitializationTest::RunTest(const FString& Parameters)
{
	for (const FString& Id : { FString(), FString(TEXT("Unknown")), FString(TEXT("Bad-Id")), FString::ChrN(65, TEXT('A')) })
	{
		FMissionTestWorld TestWorld(Id);
		TestTrue(TEXT("Invalid option gives failure result"), TestWorld.State()->HasResult() && !TestWorld.State()->GetResult().bSucceeded);
		TestFalse(TEXT("Invalid option blocks objective"), TestWorld.Target->TryInteract(TestWorld.Player));
		TestTrue(TEXT("Invalid option allows return"), TestWorld.Mode->CanReturnToStation());
		TestFalse(TEXT("Error message provided"), TestWorld.State()->GetResult().Message.IsEmpty());
	}
	for (int32 Configuration = 0; Configuration < 4; ++Configuration)
	{
		FMissionTestWorld TestWorld(TEXT("Mission_Test"), Configuration == 0 ? 0 : Configuration == 1 ? 2 : 1, Configuration == 2, Configuration == 3);
		TestTrue(TEXT("Bad setup fails without entering progress"), TestWorld.State()->GetPhase() == EWMissionPhase::Failed);
		TestTrue(TEXT("Bad setup allows return"), TestWorld.Mode->CanReturnToStation());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMissionTransferTest, "ProjectW.MissionLoop.ResultTransferConsumedOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMissionTransferTest::RunTest(const FString& Parameters)
{
	UWMissionResultTransfer* Transfer = NewObject<UWMissionResultTransfer>(NewObject<UGameInstance>());
	FWMissionResult Result;
	Result.AttemptId = FGuid::NewGuid();
	Result.MissionId = TEXT("Mission_Test");
	Result.bSucceeded = true;
	Transfer->Store(Result);
	FWMissionResult Received;
	TestTrue(TEXT("Can consume once"), Transfer->Consume(Received));
	TestTrue(TEXT("Attempt preserved"), Received.AttemptId == Result.AttemptId);
	TestFalse(TEXT("Second consumption rejected"), Transfer->Consume(Received));
	Transfer->Store(Result);
	Transfer->Clear();
	TestFalse(TEXT("Abandoned transfer cleared"), Transfer->Consume(Received));
	return true;
}
#endif
