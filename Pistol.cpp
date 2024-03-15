// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

APistol::APistol()
{
	PistolMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PistolMesh"));
	PistolMesh->SetupAttachment(RootSceneComponent);
}

void APistol::FirePressed()
{
	Super::FirePressed();

	if (bAutoFire)
	{
		bKeepFiring = true;
	}

	Fire();
	
}

void APistol::FireReleased()
{
	Super::FireReleased();

	bKeepFiring = false;
}

void APistol::PlayMagInAnimation()
{
	Super::PlayMagInAnimation();

	if (MagInLoop)
	{
		PistolMesh->PlayAnimation(MagInLoop, true);
	}
}

void APistol::PlayMagOutAnimation()
{
	Super::PlayMagOutAnimation();

	if (MagOutLoop)
	{
		PistolMesh->PlayAnimation(MagOutLoop, true);
	}
}

void APistol::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = MaxAmmo;
}

void APistol::TraceFromWeaponBarrel(FHitResult& OutHitResult)
{
	const FVector Start = BarrelPoint->GetComponentLocation();
	const FVector End = Start + BarrelPoint->GetForwardVector() * 50000.f;

	
	GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
	
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.f);
}

void APistol::Fire()
{
	if (CurrentAmmo > 0 && bCanFire) // TODO: Make it so when it shoots it gets the hand and adds a random rotation upwards to simulate recoil and then it interps back to default
	{
		bCanFire = false;
		CurrentAmmo--;
		if (FireAnimation)
		{
			PistolMesh->PlayAnimation(FireAnimation, false);
		}
		FHitResult FireHitResult;
		TraceFromWeaponBarrel(FireHitResult);

		GetWorldTimerManager().SetTimer(FireDelayTimer, this, &APistol::FireDelayTimerFinished, FireDelayTime);
	}
}

void APistol::FireDelayTimerFinished()
{
	bCanFire = true;

	if (bKeepFiring)
	{
		Fire();
	}
}
