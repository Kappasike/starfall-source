// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "MotionControllerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Shotgun is a special weapon because you don't ever need to reload, you just need to cock back the bolt after every shot

AShotgun::AShotgun()
{
	ShotgunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShotgunMesh"));
	ShotgunMesh->SetupAttachment(RootSceneComponent);

	ShotgunBolt = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShotgunBolt"));
	ShotgunBolt->SetupAttachment(RootGripPoint);

	BoltEndPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BoltEndPoint"));
	BoltEndPoint->SetupAttachment(RootSceneComponent);
}

void AShotgun::Tick(float DeltaTime)
{
	UpdateBoltSlide();
}

void AShotgun::FirePressed()
{
	Super::FirePressed();

	Fire();
}

void AShotgun::FireReleased()
{
	Super::FireReleased();
}

void AShotgun::Fire()
{
	if (ShotgunState == EShotgunState::ESS_CanFire)
	{
		ShotgunState = EShotgunState::ESS_BoltStart;
		if (FireAnimation)
		{
			ShotgunMesh->PlayAnimation(FireAnimation, false);
		}
		FHitResult FireHitResult;
		TraceFromWeaponBarrel(FireHitResult);
	}
}

void AShotgun::TraceFromWeaponBarrel(FHitResult& OutHitResult)
{
	const FVector Start = BarrelPoint->GetComponentLocation();
	const FVector End = Start + BarrelPoint->GetForwardVector() * 50000.f;


	GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.f);
}

// Important bolt slide values, 15 is start/end point, 6 is turn around point

void AShotgun::UpdateBoltSlide()
{
	if (bIsAlreadyEquipped && GripMotionController && ShotgunState != EShotgunState::ESS_CanFire)
	{
		/*FRotator GunRotation = RootSceneComponent->GetComponentRotation();
		FVector NewLocation = GripMotionController->GetRelativeLocation();
		GunRotation.RotateVector(NewLocation);*/
		float NewDistance = UKismetMathLibrary::Vector_Distance(GripMotionController->GetComponentLocation(), BoltEndPoint->GetComponentLocation());

		DeltaHandPositionX = NewDistance - PreviousHandPositionX;
		//PreviousHandPositionX = NewLocation.X;
		PreviousHandPositionX = NewDistance;

		if (ShotgunState == EShotgunState::ESS_BoltStart && DeltaHandPositionX < 0)
		{
			float NewRelativePositionX = FMath::Clamp(RootGripPoint->GetRelativeLocation().Y + DeltaHandPositionX, 6.f, 15.f);
			RootGripPoint->SetRelativeLocation(FVector(0.f, NewRelativePositionX, 0.42f));
			if (RootGripPoint->GetRelativeLocation().Y < 6.15f) // this means you've cocked back bolt far enough now it must go forward
			{
				ShotgunState = EShotgunState::ESS_BoltEnd;
				if (InitialBoltSoundCue)
				{
					UGameplayStatics::PlaySoundAtLocation(this, InitialBoltSoundCue, RootGripPoint->GetComponentLocation());
				}
			}
		}
		else if (ShotgunState == EShotgunState::ESS_BoltEnd && DeltaHandPositionX > 0)
		{
			float NewRelativePositionX = FMath::Clamp(RootGripPoint->GetRelativeLocation().Y + DeltaHandPositionX, 6.f, 15.f);
			RootGripPoint->SetRelativeLocation(FVector(0.f, NewRelativePositionX, 0.42f));
			if (RootGripPoint->GetRelativeLocation().Y > 14.85f) // this means you've cocked forward bolt far enough now you can fire again
			{
				ShotgunState = EShotgunState::ESS_CanFire;
				if (FinalBoltSoundCue)
				{
					UGameplayStatics::PlaySoundAtLocation(this, FinalBoltSoundCue, RootGripPoint->GetComponentLocation());
				}
			}
		}
	}
}
