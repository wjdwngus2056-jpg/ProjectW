// Copyright (C) 2026 TeamW. All Rights Reserved.
#include "ProjectW/Mission/Framework/WMissionGameMode.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "ProjectW/AbilitySystem/Components/WSkillComponent.h"
#include "ProjectW/Actors/Player/WPlayerCharacter.h"
#include "ProjectW/Actors/WorldObjects/WMissionPanel.h"
#include "ProjectW/Actors/WorldObjects/WPortal.h"
#include "ProjectW/Actors/WorldObjects/WDevelopmentMissionObjective.h"
#include "ProjectW/Core/Framework/WPlayerController.h"
#include "ProjectW/FireStation/Framework/WFireStationGameMode.h"
#include "ProjectW/Mission/Framework/WMissionGameState.h"
#include "ProjectW/Mission/Travel/WMissionResultTransfer.h"
#include "ProjectW/UI/Mission/WMissionLoopHUD.h"

namespace
{
	/** 실제 -game 월드를 사용한다. 맵/캐릭터/컨트롤러를 테스트용으로 대체하지 않는다. */
	class FWMissionRoundTripCommand : public IAutomationLatentCommand
	{
	public:
		explicit FWMissionRoundTripCommand(FAutomationTestBase* InTest) : Test(InTest) {}
		virtual bool Update() override
		{
			const double Now = FPlatformTime::Seconds();
			if (Now > Deadline) { Test->AddError(FString::Printf(TEXT("Round trip timed out at stage %d, round %d"), Stage, Round)); return true; }
			if (Now < NextAction) return false;
			UWorld* World = nullptr;
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::Game && Context.World() && Context.World()->HasBegunPlay()) { World = Context.World(); break; }
			}
			if (!World) return false;
			AWPlayerController* Controller = nullptr;
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				AWPlayerController* Candidate = Cast<AWPlayerController>(It->Get());
				if (Candidate && Candidate->IsLocalController() && Candidate->GetLocalPlayer()) { Controller = Candidate; break; }
			}
			AWPlayerCharacter* Player = Controller ? Cast<AWPlayerCharacter>(Controller->GetPawn()) : nullptr;
			if (!Controller || !Player) return false;
			auto Next = [this, Now](int32 NewStage) { Stage = NewStage; NextAction = Now + 0.25; Deadline = Now + 60.0; };
			auto Interact = [this, Controller, Player](AActor* Target)
			{
				if (!Target) { Test->AddError(TEXT("Missing interaction target in shipped map")); return false; }
				Player->SetActorLocation(Target->GetActorLocation() + FVector(-110, 0, 90), false, nullptr, ETeleportType::TeleportPhysics);
				if (!Test->TestEqual(TEXT("Existing nearest-target interaction route"), Player->FindInteractionTarget(), Target)) return false;
				UEnhancedInputLocalPlayerSubsystem* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
				if (!Input || !Controller->InteractionAction) { Test->AddError(TEXT("Interaction input asset missing")); return false; }
				Input->InjectInputForAction(Controller->InteractionAction, FInputActionValue(true), {}, {});
				return true;
			};
			if (Stage == 0)
			{
				AWFireStationGameMode* Station = World->GetAuthGameMode<AWFireStationGameMode>();
				if (!Station) return false;
				Test->TestNotNull(TEXT("Station HUD available"), Cast<AWMissionLoopHUD>(Controller->GetHUD()));
				Test->TestTrue(TEXT("Station selection is fresh"), Station->GetSelectedMissionId().IsNone());
				if (Round > 0)
				{
					Test->TestTrue(TEXT("Station received confirmed attempt"), Station->HasLastResult() && Station->GetLastResult().AttemptId == PreviousAttempt);
					Test->TestEqual(TEXT("Station received expected outcome"), Station->GetLastResult().bSucceeded, Round <= 3);
					FWMissionResult Consumed;
					Test->TestFalse(TEXT("Transfer was consumed once"), World->GetGameInstance()->GetSubsystem<UWMissionResultTransfer>()->Consume(Consumed));
				}
				if (Round == 5) { Test->AddInfo(TEXT("Three success round trips, unknown MissionId and missing MissionId returned to station.")); return true; }
				if (Round >= 3)
				{
					UGameplayStatics::OpenLevel(World, TEXT("/Game/Level/L_Prototype_Mission"), true, Round == 3 ? TEXT("MissionId=Unknown") : TEXT(""));
					Next(3); return false;
				}
				TActorIterator<AWMissionPanel> Panel(World);
				if (!Interact(Panel ? *Panel : nullptr)) return true;
				Next(1); return false;
			}
			if (Stage == 1)
			{
				AWFireStationGameMode* Station = World->GetAuthGameMode<AWFireStationGameMode>();
				if (!Station) return false;
				if (!Test->TestEqual(TEXT("Input selected existing Mission_Test row"), Station->GetSelectedMissionId(), FName(TEXT("Mission_Test")))) return true;
				TActorIterator<AWPortal> Portal(World);
				if (!Interact(Portal ? *Portal : nullptr)) return true;
				Next(2); return false;
			}
			AWMissionGameState* State = World->GetGameState<AWMissionGameState>();
			if (!State) return false;
			if (Stage == 2)
			{
				Test->TestTrue(TEXT("Mission starts in progress"), State->GetPhase() == EWMissionPhase::InProgress);
				Test->TestEqual(TEXT("MissionId read from portal options"), State->GetMissionId(), FName(TEXT("Mission_Test")));
				Test->TestEqual(TEXT("Redeploy resets progress"), State->GetCompletedObjectives(), 0);
				Test->TestFalse(TEXT("Redeploy has no result"), State->HasResult());
				Test->TestTrue(TEXT("Redeploy issues fresh attempt ID"), State->GetAttemptId().IsValid() && State->GetAttemptId() != PreviousAttempt);
				UWSkillComponent* Skills = Player->FindComponentByClass<UWSkillComponent>();
				Test->TestTrue(TEXT("Existing Bolt fires on each new attempt"), Skills && Skills->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::RightVector).IsAccepted());
				TActorIterator<AWDevelopmentMissionObjective> Objective(World);
				if (!Interact(Objective ? *Objective : nullptr)) return true;
				Next(3); return false;
			}
			if (Stage == 3)
			{
				if (!State->HasResult()) return false;
				Test->TestEqual(TEXT("Confirmed expected result"), State->GetResult().bSucceeded, Round < 3);
				Test->TestEqual(TEXT("Confirmed objective count"), State->GetCompletedObjectives(), Round < 3 ? 1 : 0);
				Test->TestFalse(TEXT("Result contains explanation"), State->GetResult().Message.IsEmpty());
				PreviousAttempt = State->GetAttemptId();
				TActorIterator<AWDevelopmentMissionObjective> Objective(World);
				if (Objective) Test->TestFalse(TEXT("Repeated completion rejected"), Objective->TryInteract(Player));
				Test->TestTrue(TEXT("Result blocks new Bolt requests"), Player->FindComponentByClass<UWSkillComponent>()->TryActivateSlot(EWAbilitySlot::BasicAttack, FVector::RightVector).Status == EWAbilityRequestStatus::Unavailable);
				if (FParse::Param(FCommandLine::Get(), TEXT("MissionLoopScreenshots")))
				{
					FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / FString::Printf(TEXT("MissionLoopScreenshots/Round%dResult.png"), Round), true, false);
				}
				Next(4); return false;
			}
			if (Stage == 4)
			{
				AWMissionLoopHUD* HUD = Cast<AWMissionLoopHUD>(Controller->GetHUD());
				if (!Test->TestNotNull(TEXT("Result HUD available"), HUD)) return true;
				// 결과 UI 클릭 콜백으로 복귀 요청을 보낸다. 규칙은 GameMode가 다시 검증한다.
				HUD->NotifyHitBoxClick(TEXT("ReturnToStation"));
				Test->TestTrue(TEXT("Return accepted"), State->GetPhase() == EWMissionPhase::Returning);
				Test->TestFalse(TEXT("Repeated return rejected"), World->GetAuthGameMode<AWMissionGameMode>()->TryReturnToStation(Controller));
				++Round;
				Next(0); return false;
			}
			return false;
		}
	private:
		FAutomationTestBase* Test;
		int32 Stage = 0;
		int32 Round = 0;
		FGuid PreviousAttempt;
		double NextAction = 0;
		double Deadline = FPlatformTime::Seconds() + 60.0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMissionPlayRoundTripTest, "ProjectW.MissionLoop.PlayRoundTrip",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FWMissionPlayRoundTripTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FWMissionRoundTripCommand(this));
	return true;
}
#endif
