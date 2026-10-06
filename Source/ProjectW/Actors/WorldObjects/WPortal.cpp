// Copyright (C) 2026 TeamW. All Rights Reserved.


#include "WPortal.h"


// Sets default values
AWPortal::AWPortal()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWPortal::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

