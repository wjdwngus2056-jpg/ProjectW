// Copyright (C) 2026 TeamW. All Rights Reserved.
#include "WRescueTarget.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "ProjectW/Mission/Objectives/WMissionObjectiveComponent.h"
#include "UObject/ConstructorHelpers.h"

AWRescueTarget::AWRescueTarget()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RescueRoot")));
	Objective = CreateDefaultSubobject<UWMissionObjectiveComponent>(TEXT("Objective"));
	Objective->ObjectiveId = TEXT("RescueCitizen");
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Body->SetupAttachment(RootComponent);
	Head->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere"));
	Body->SetStaticMesh(Cylinder.Object);
	Body->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.0f));
	Head->SetStaticMesh(Sphere.Object);
	Head->SetRelativeLocation(FVector(0, 0, 75));
	Head->SetRelativeScale3D(FVector(0.4f));
	// 후보 탐색에는 포함하되 이동·커서 조준·투사체를 막는 판정은 추가하지 않는다.
	for (UStaticMeshComponent* Mesh : { Body.Get(), Head.Get() })
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionObjectType(ECC_WorldDynamic);
		Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		Mesh->SetGenerateOverlapEvents(true);
	}
	StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PrototypeStatus"));
	StatusText->SetupAttachment(RootComponent);
	StatusText->SetRelativeLocation(FVector(0, 0, 135));
	StatusText->SetRelativeRotation(FRotator(0, 180, 0));
	StatusText->SetHorizontalAlignment(EHTA_Center);
	StatusText->SetWorldSize(38.0f);
	UpdateRescueVisuals();
}

void AWRescueTarget::BeginPlay()
{
	Super::BeginPlay();
	// 새 월드의 등록 상태를 읽어 임시 표시도 미구조부터 시작한다.
	UpdateRescueVisuals();
}

bool AWRescueTarget::IsRescued() const
{
	return Objective->IsCompleted();
}

bool AWRescueTarget::CanInteract(AActor* Interactor) const
{
	const APawn* Pawn = Cast<APawn>(Interactor);
	// 상호작용 후보·HUD의 안내 여부와 관계없이 실제 요청자의 현재 거리와 구조 상태를 검사한다.
	return IsValid(Pawn) && Pawn->GetWorld() == GetWorld() && Pawn->IsPlayerControlled() &&
		FVector::DistSquared(GetActorLocation(), Pawn->GetActorLocation()) <= FMath::Square(InteractionDistance) &&
		Objective->CanReportCompletion();
}

bool AWRescueTarget::TryInteract(AActor* Interactor)
{
	if (!CanInteract(Interactor) || !Objective->TryComplete()) return false;
	// 수락된 최초 전이에서만 피드백을 갱신한다. 구조 완료는 미션 종료와 별개다.
	UpdateRescueVisuals();
	return true;
}

FText AWRescueTarget::GetInteractionText(AActor* Interactor) const
{
	return IsRescued() ? NSLOCTEXT("Rescue", "Rescued", "시민 구조 완료") : NSLOCTEXT("Rescue", "Rescue", "시민 구조");
}

void AWRescueTarget::UpdateRescueVisuals()
{
	StatusText->SetText(FText::FromString(IsRescued() ? TEXT("OK") : TEXT("SOS")));
	StatusText->SetTextRenderColor(IsRescued() ? FColor::Green : FColor::Yellow);
}
