// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "MotionControllerComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootComponent"));
	SetRootComponent(RootSceneComponent);

	RootGripPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RootGripPoint"));
	RootGripPoint->SetupAttachment(RootSceneComponent);
	RootGripPoint->SetMobility(EComponentMobility::Movable);

	LeftGripPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftGripPoint"));
	LeftGripPoint->SetupAttachment(RootGripPoint);
	LeftGripPoint->SetMobility(EComponentMobility::Movable);

	RightGripPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightGripPoint"));
	RightGripPoint->SetupAttachment(RootGripPoint);
	RightGripPoint->SetMobility(EComponentMobility::Movable);

	BarrelPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPoint"));
	BarrelPoint->SetupAttachment(RootSceneComponent);

	ReloadPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ReloadPoint"));
	ReloadPoint->SetupAttachment(RootSceneComponent);
}

void AWeapon::PlayMagInAnimation()
{
	
}

void AWeapon::PlayMagOutAnimation()
{

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AWeapon::FirePressed()
{

}

void AWeapon::FireReleased()
{

}

void AWeapon::Equip()
{
	RootSceneComponent->SetSimulatePhysics(false);
	bIsAlreadyEquipped = true;
}

void AWeapon::Drop()
{
	RootSceneComponent->SetSimulatePhysics(true);
	bIsAlreadyEquipped = false;
}
