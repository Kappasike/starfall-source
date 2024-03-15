// Fill out your copyright notice in the Description page of Project Settings.


#include "Vault.h"
#include "Components/BoxComponent.h"
#include "../Character/VRBaseCharacter.h"
#include "MotionControllerComponent.h"
#include "Kismet/KismetMathLibrary.h"

AVault::AVault()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootSceneComponent);

	FirstGripLocation = CreateDefaultSubobject<USceneComponent>(TEXT("FirstGripLocation"));
	FirstGripLocation->SetupAttachment(RootSceneComponent);

	SecondGripLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SecondGripLocation"));
	SecondGripLocation->SetupAttachment(RootSceneComponent);

	DefaultGripLocation = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultGripLocation"));
	DefaultGripLocation->SetupAttachment(RootSceneComponent);

	VaultMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VaultMesh"));
	VaultMesh->SetupAttachment(RootSceneComponent);

	TargetPushComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TargetPushComponent"));
	TargetPushComponent->SetupAttachment(RootSceneComponent);
}

void AVault::SetPawnCollision(bool bEnableCollision)
{
	/*if (bEnableCollision)
	{
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	}
	else
	{
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}*/
}

void AVault::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ClimbMovementUpdate();
}

void AVault::ClimbMovementUpdate()
{
	if (HandsEquipped == 0 && OwnerCharacter && OwnerCharacter->ParkourMovementState == EParkourMovementState::EMS_Climb)
	{
		OwnerCharacter->ParkourMovementState = EParkourMovementState::EMS_Unoccupied;
	}
	else if (HandsEquipped == 1 && FirstMotionController)
	{
		if (PreviousFirstControllerPos == FVector(0.f, 0.f, 0.f))
		{
			PreviousFirstControllerPos = FirstMotionController->GetRelativeLocation();
			return;
		}
		const FVector DeltaPosition = FirstMotionController->GetRelativeLocation() - PreviousFirstControllerPos;
		PreviousFirstControllerPos = FirstMotionController->GetRelativeLocation();

		TargetPushComponent->AddWorldOffset(-DeltaPosition*2.f);
		FVector LaunchDirection = TargetPushComponent->GetComponentLocation() - OwnerCharacter->GetActorLocation();

		OwnerCharacter->LaunchCharacter(LaunchDirection, false, false);
	}
	else if (HandsEquipped == 2 && FirstMotionController && SecondMotionController)
	{
		if (PreviousFirstControllerPos == FVector(0.f, 0.f, 0.f))
		{
			PreviousFirstControllerPos = FirstMotionController->GetRelativeLocation();
			return;
		}
		if (PreviousSecondControllerPos == FVector(0.f, 0.f, 0.f))
		{
			PreviousSecondControllerPos = SecondMotionController->GetRelativeLocation();
			return;
		}
		const FVector FirstDeltaPosition = FirstMotionController->GetRelativeLocation() - PreviousFirstControllerPos;
		const FVector SecondDeltaPosition = SecondMotionController->GetRelativeLocation() - PreviousSecondControllerPos;
		PreviousFirstControllerPos = FirstMotionController->GetRelativeLocation();
		PreviousSecondControllerPos = SecondMotionController->GetRelativeLocation();

		const FVector LerpedDeltaPosition = UKismetMathLibrary::VLerp(FirstDeltaPosition, SecondDeltaPosition, 0.5f);

		TargetPushComponent->AddWorldOffset(-LerpedDeltaPosition*3.f);
		FVector LaunchDirection = TargetPushComponent->GetComponentLocation() - OwnerCharacter->GetActorLocation();

		OwnerCharacter->LaunchCharacter(LaunchDirection, false, false);
	}
}