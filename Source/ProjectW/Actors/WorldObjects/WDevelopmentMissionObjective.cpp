// Copyright (C) 2026 TeamW. All Rights Reserved.
#include "WDevelopmentMissionObjective.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "ProjectW/Mission/Framework/WMissionGameMode.h"
#include "UObject/ConstructorHelpers.h"

AWDevelopmentMissionObjective::AWDevelopmentMissionObjective()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectiveMesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube"));
	Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
}

bool AWDevelopmentMissionObjective::CanInteract(AActor* Interactor) const
{
	const AWMissionGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<AWMissionGameMode>() : nullptr;
	return Mode && Mode->CanCompleteDevelopmentObjective(this, Interactor);
}

bool AWDevelopmentMissionObjective::TryInteract(AActor* Interactor)
{
	AWMissionGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<AWMissionGameMode>() : nullptr;
	return Mode && Mode->TryCompleteDevelopmentObjective(this, Interactor);
}

FText AWDevelopmentMissionObjective::GetInteractionText(AActor* Interactor) const
{
	return NSLOCTEXT("Mission", "DevelopmentComplete", "개발용 목표 완료");
}
