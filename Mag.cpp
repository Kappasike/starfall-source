// Fill out your copyright notice in the Description page of Project Settings.


#include "Mag.h"

AMag::AMag()
{
	PrimaryActorTick.bCanEverTick = true;

	MagRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MagRootComponent"));
	SetRootComponent(MagRootComponent);

	MagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagMesh"));
	MagMesh->SetupAttachment(MagRootComponent);
}

void AMag::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMag::Drop()
{
	MagMesh->SetSimulatePhysics(true);
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AMag::DestroyTimerFinished, DestroyTime);
}

void AMag::DestroyTimerFinished()
{
	Destroy();
}