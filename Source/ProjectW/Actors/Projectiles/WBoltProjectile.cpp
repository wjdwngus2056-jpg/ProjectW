// Copyright (C) 2026 TeamW. All Rights Reserved.

#include "WBoltProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectW/Actors/Collision/WCollisionChannels.h"
#include "UObject/ConstructorHelpers.h"

AWBoltProjectile::AWBoltProjectile()
{
	InitialLifeSpan = 2.0f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(12.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(WCollisionChannels::SkillProjectile);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->SetCollisionResponseToChannel(WCollisionChannels::PlayerCharacter, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(WCollisionChannels::SkillProjectile, ECR_Ignore);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionObjectType(WCollisionChannels::SkillProjectile);
	Mesh->SetRelativeScale3D(FVector(0.24));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Collision);
	Movement->Velocity = FVector::ForwardVector;
	Movement->InitialSpeed = 1000.0f;
	Movement->bInitialVelocityInLocalSpace = true;
	Movement->ProjectileGravityScale = 0.0f;
	Movement->bRotationFollowsVelocity = true;
	Movement->OnProjectileStop.AddDynamic(this, &AWBoltProjectile::OnProjectileStopped);
}

void AWBoltProjectile::SetDamage(float InDamage)
{
	Damage = InDamage;
}

void AWBoltProjectile::BeginPlay()
{
	if (AActor* Caster = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(Caster, true);
	}

	Super::BeginPlay();
}

void AWBoltProjectile::OnProjectileStopped(const FHitResult& Hit)
{
	if (bHasImpacted)
	{
		return;
	}
	bHasImpacted = true;
	AActor* HitActor = Hit.GetActor();
	if (IsValid(HitActor) && HitActor != GetOwner() && Damage > 0.0f)
	{
		UGameplayStatics::ApplyPointDamage(
			HitActor, Damage, GetActorForwardVector(), Hit, GetInstigatorController(), this, nullptr);
	}

	Destroy();
}
