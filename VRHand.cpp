// Fill out your copyright notice in the Description page of Project Settings.


#include "VRHand.h"
#include "Components/SphereComponent.h"
#include "../Items/Weapon.h"
#include "../HandAnimInstance.h" // why is this not in this folder idk, fix later
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../Items/Mag.h"
#include "Sound/SoundCue.h"
#include "../Items/Shotgun.h"
#include "../Obstacles/Vault.h"
#include "../Character/VRBaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AVRHand::AVRHand()
{
	PrimaryActorTick.bCanEverTick = true;

	HandRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HandRoot"));
	SetRootComponent(HandRoot);

	HandAttachment = CreateDefaultSubobject<USceneComponent>(TEXT("HandAttch"));
	HandAttachment->SetupAttachment(HandRoot);

	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandMesh"));
	HandMesh->SetupAttachment(HandAttachment);

	HandCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HandCollision"));
	HandCollision->SetupAttachment(HandAttachment);
}

void AVRHand::BeginPlay()
{
	Super::BeginPlay();

	HandMeshAnimInstance = Cast<UHandAnimInstance>(HandMesh->GetAnimInstance()); // hip hip, hooray!
	
	HandCollision->OnComponentBeginOverlap.AddDynamic(this, &AVRHand::OnHandCollisionBeginOverlap);
	HandCollision->OnComponentEndOverlap.AddDynamic(this, &AVRHand::OnHandCollisionEndOverlap);

	HandTarget = HandRoot;
}

void AVRHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AlignHandWithTarget();
	CheckMagInsert();

	/*if (OverlappingVault && OwnerCharacter)
	{
		FVector DistanceToVault = UKismetMathLibrary::Subtract_VectorVector(OwnerCharacter->VRCamera->GetComponentLocation(), OverlappingVault->DefaultGripLocation->GetComponentLocation());
		UKismetMathLibrary::Vector_Normalize(DistanceToVault);
		const float DP = UKismetMathLibrary::Dot_VectorVector(DistanceToVault, OverlappingVault->DefaultGripLocation->GetForwardVector());
		if (DP > 0) // if (-), player is in front of vault
		{
			GEngine->AddOnScreenDebugMessage(14, 0, FColor::Blue, FString::Printf(TEXT("DotProduct: %f"), DP));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(14, 0, FColor::Red, FString::Printf(TEXT("DotProduct: %f"), DP));
		}
		float VaultYaw = OverlappingVault->DefaultGripLocation->GetComponentRotation().GetDenormalized().Yaw;
		float HandYaw = GetActorRotation().GetDenormalized().Yaw;
		GEngine->AddOnScreenDebugMessage(14, 0, FColor::Green, FString::Printf(TEXT("VaultYaw: %f"), VaultYaw));
		GEngine->AddOnScreenDebugMessage(15, 0, FColor::Red, FString::Printf(TEXT("HandYaw: %f"), HandYaw));
	}*/
}

void AVRHand::OnHandCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AWeapon* OverlappedWeapon = Cast<AWeapon>(OtherActor);
	AVault* OverlappedVault = Cast<AVault>(OtherActor);
	if (OverlappedWeapon)
	{
		OverlappingWeapon = OverlappedWeapon;
	}
	else if (OverlappedVault)
	{
		OverlappingVault = OverlappedVault;
	}
	else if (OtherComp->ComponentHasTag(FName("AmmoPouch")))
	{
		bCanGrabAmmo = true;
	}
}

void AVRHand::OnHandCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Reason I'm seeing if it's valid is in case I stop colliding with something that's
	// not a weapon it won't make a weapon I am overlapping with nullptr
	AWeapon* OverlappedWeapon = Cast<AWeapon>(OtherActor);
	AVault* OverlappedVault = Cast<AVault>(OtherActor);
	if (OverlappedWeapon)
	{
		OverlappingWeapon = nullptr;
	}
	else if (OverlappedVault)
	{
		OverlappingVault = nullptr;
	}
	else if (OtherComp->ComponentHasTag(FName("AmmoPouch")))
	{
		bCanGrabAmmo = false;
	}
}

// I've been the archer, I've been the prey, who could ever leave me darling, but who could stay? Dark side, I search for your dark side

void AVRHand::AlignHandWithTarget()
{
	if (bHandRotInterp && OtherHand) // change rotation of hand to aim halfway to position of gripped hand
	{
		// set look at rotation for motioncontroller position then interp between that rotation and current hand root rotation, prepare for awesomeness
		const FRotator InterpRotation = UKismetMathLibrary::FindLookAtRotation(HandRoot->GetComponentLocation(), OtherHand->HandRoot->GetComponentLocation());
		FRotator NewRotation = UKismetMathLibrary::NormalizedDeltaRotator(InterpRotation, InitialHandRotation);
		HandAttachment->SetWorldRotation(InterpRotation + InitialHandRotation);
	}
	else if (HandTarget)
	{
		HandAttachment->SetWorldRotation(HandTarget->GetComponentRotation());
	}
}

/**
* Hand Inputs
*/

void AVRHand::GripPressed()
{
	if (HandMeshAnimInstance)
	{
		HandMeshAnimInstance->BottomGrip = true;
	}
	
	GrabWeapon();
}

void AVRHand::GripReleased()
{
	if (HandMeshAnimInstance)
	{
		HandMeshAnimInstance->BottomGrip = false;

	}

	DropWeapon();
}

void AVRHand::TriggerPressed()
{
	if (HandMeshAnimInstance)
	{
		HandMeshAnimInstance->IndexFinger = true;
	}

	FireWeaponPressed();
}

void AVRHand::TriggerReleased()
{
	if (HandMeshAnimInstance)
	{
		HandMeshAnimInstance->IndexFinger = false;
	}

	FireWeaponReleased();
}

void AVRHand::ThumbPressed()
{
	if (HandMeshAnimInstance)
	{
		HandMeshAnimInstance->Thumb = true;
	}
}

void AVRHand::ThumbReleased()
{
	if (HandMeshAnimInstance)
	{
		HandMeshAnimInstance->Thumb = false;
	}
}

void AVRHand::GrabWeapon()
{
	if (OverlappingWeapon && !OverlappingWeapon->bIsAlreadyEquipped && EquippedWeapon == nullptr)
	{
		FName TargetSocket = GetAppropriateSocket();
		EquippedWeapon = OverlappingWeapon;
		EquippedWeapon->Equip();
		OverlappingWeapon = nullptr;
		if (HandMesh)
		{
			EquippedWeapon->AttachToComponent(HandMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocket);
		}
		if (HandMeshAnimInstance)
		{
			HandMeshAnimInstance->bWeaponIsEquipped = true;
		}
	}
	else if (OverlappingWeapon && OverlappingWeapon->bIsAlreadyEquipped && EquippedWeapon == nullptr) // this means to grab the grip on the weapon
	{
		// first check if the hand is close to the reload point because if so then we grab the mag instead of gripping the weapon, this is cheaper than have 2 hitboxes
		const float DistanceToReloadPoint = UKismetMathLibrary::Vector_DistanceSquared(HandRoot->GetComponentLocation(), OverlappingWeapon->ReloadPoint->GetComponentLocation());
		if (DistanceToReloadPoint < 100.f && OverlappingWeapon->MagType)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = this;
			CurrentMag = GetWorld()->SpawnActor<AMag>(OverlappingWeapon->MagType, SpawnParameters);
			CurrentMag->AttachToComponent(HandMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "LeftMagSocket");
			CurrentMag->bEmpty = true;
			OverlappingWeapon->bHasMag = false;
			if (CurrentMag->OutsertSoundCue)
			{
				UGameplayStatics::PlaySoundAtLocation(this, CurrentMag->OutsertSoundCue, CurrentMag->GetActorLocation());
			}
			OverlappingWeapon->CurrentAmmo = 0;
			OverlappingWeapon->PlayMagOutAnimation();
		}
		else // this grips the hand based on what hand and weapon it is, will make these all into separate functions once everything is completed
		{
			if (bIsRightHand)
			{
				HandTarget = OverlappingWeapon->RightGripPoint;
				HandAttachment->AttachToComponent(OverlappingWeapon->RightGripPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			else
			{
				HandAttachment->AttachToComponent(OverlappingWeapon->LeftGripPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				HandTarget = OverlappingWeapon->LeftGripPoint;
			}
			if (OtherHand)
			{
				FRotator InitialInterpRotation = UKismetMathLibrary::FindLookAtRotation(OtherHand->HandRoot->GetComponentLocation(), HandRoot->GetComponentLocation());
				FRotator NewInitialHandRotationDelta = OtherHand->HandRoot->GetComponentRotation() - InitialInterpRotation;
				OtherHand->InitialHandRotation = NewInitialHandRotationDelta;
				OtherHand->bHandRotInterp = true;
			}

			GrippedWeapon = OverlappingWeapon; // GrippedWeapon is useless for now but it could be useful later
			AShotgun* GrippedShotgun = Cast<AShotgun>(OverlappingWeapon);
			if (GrippedShotgun && ParentMotionController)
			{
				GrippedShotgun->GripMotionController = ParentMotionController;
			}
		}
	}
	else if (OverlappingVault)
	{
		if (OverlappingVault->HandsEquipped == 0) // first hand to be equipped
		{
			if (OwnerCharacter)
			{
				OwnerCharacter->SetVaultMovementState(true);
				OverlappingVault->TargetPushComponent->SetWorldLocation(OwnerCharacter->GetActorLocation());
				OverlappingVault->OwnerCharacter = OwnerCharacter;
				OverlappingVault->FirstMotionController = ParentMotionController;
				OverlappingVault->SetPawnCollision(false);
			}
			OverlappingVault->HandsEquipped++; // now 1 hand is equipped
			CurrentVault = OverlappingVault;
			HandTarget = CurrentVault->FirstGripLocation;
			
			// Sets location of new location for hand to grip
			const float NewGripPositionX = UKismetMathLibrary::Vector_Distance2D(CurrentVault->DefaultGripLocation->GetComponentLocation(), HandRoot->GetComponentLocation());
			
			
			// Sets rotation for hand to grip based on if your in front of or behind object and what hand, left or right
			FRotator NewGripRotation;
			FVector2D NewGripLocation;
			if (OverlappingVault && OwnerCharacter)
			{
				FVector DistanceToVault = UKismetMathLibrary::Subtract_VectorVector(OwnerCharacter->VRCamera->GetComponentLocation(), OverlappingVault->DefaultGripLocation->GetComponentLocation());
				UKismetMathLibrary::Vector_Normalize(DistanceToVault);
				const float DP = UKismetMathLibrary::Dot_VectorVector(DistanceToVault, OverlappingVault->DefaultGripLocation->GetForwardVector());
				if (DP < 0) // if (-), player is in front of vault
				{
					GEngine->AddOnScreenDebugMessage(14, 2, FColor::Blue, "In-Front");
					if (bIsRightHand)
					{
						NewGripRotation = OverlappingVault->InFrontRightHandRot;
						NewGripLocation = OverlappingVault->InFrontRightHandPos;
					}
					else
					{
						NewGripRotation = OverlappingVault->InFrontLeftHandRot;
						NewGripLocation = OverlappingVault->InFrontLeftHandPos;
					}
				}
				else // player is behind vault
				{
					GEngine->AddOnScreenDebugMessage(14, 2, FColor::Red, "Behind");
					if (bIsRightHand)
					{
						NewGripRotation = OverlappingVault->BehindRightHandRot;
						NewGripLocation = OverlappingVault->BehindRightHandPos;
					}
					else
					{
						NewGripRotation = OverlappingVault->BehindLeftHandRot;
						NewGripLocation = OverlappingVault->BehindRightHandPos;
					}
				}
			}
			CurrentVault->FirstGripLocation->SetRelativeRotation(NewGripRotation);
			CurrentVault->FirstGripLocation->SetRelativeLocation(FVector(-NewGripPositionX, NewGripLocation.X, NewGripLocation.Y));

			HandAttachment->AttachToComponent(CurrentVault->FirstGripLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
		else if (OverlappingVault->HandsEquipped == 1) // one is already equipped so use other component
		{
			OverlappingVault->HandsEquipped++; // now 2 hands are equipped
			OverlappingVault->SecondMotionController = ParentMotionController;
			CurrentVault = OverlappingVault;
			HandTarget = CurrentVault->SecondGripLocation;
			const float NewGripPositionX = UKismetMathLibrary::Vector_Distance2D(CurrentVault->DefaultGripLocation->GetComponentLocation(), HandRoot->GetComponentLocation());
			
			FRotator NewGripRotation;
			FVector2D NewGripLocation;
			if (OverlappingVault && OwnerCharacter)
			{
				FVector DistanceToVault = UKismetMathLibrary::Subtract_VectorVector(OwnerCharacter->VRCamera->GetComponentLocation(), OverlappingVault->DefaultGripLocation->GetComponentLocation());
				UKismetMathLibrary::Vector_Normalize(DistanceToVault);
				const float DP = UKismetMathLibrary::Dot_VectorVector(DistanceToVault, OverlappingVault->DefaultGripLocation->GetForwardVector());
				if (DP < 0) // if (-), player is in front of vault
				{
					//GEngine->AddOnScreenDebugMessage(14, 2, FColor::Blue, "In-Front");
					if (bIsRightHand)
					{
						NewGripRotation = OverlappingVault->InFrontRightHandRot;
						NewGripLocation = OverlappingVault->InFrontRightHandPos;
					}
					else
					{
						NewGripRotation = OverlappingVault->InFrontLeftHandRot;
						NewGripLocation = OverlappingVault->InFrontLeftHandPos;
					}
				}
				else // player is behind vault
				{
					//GEngine->AddOnScreenDebugMessage(14, 2, FColor::Red, "Behind");
					if (bIsRightHand)
					{
						NewGripRotation = OverlappingVault->BehindRightHandRot;
						NewGripLocation = OverlappingVault->BehindRightHandPos;
					}
					else
					{
						NewGripRotation = OverlappingVault->BehindLeftHandRot;
						NewGripLocation = OverlappingVault->BehindRightHandPos;
					}
				}
			}
			CurrentVault->SecondGripLocation->SetRelativeRotation(NewGripRotation);
			CurrentVault->SecondGripLocation->SetRelativeLocation(FVector(-NewGripPositionX, NewGripLocation.X, NewGripLocation.Y));
			
			HandAttachment->AttachToComponent(CurrentVault->SecondGripLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}
	else if (bCanGrabAmmo && OtherHand && OtherHand->EquippedWeapon)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		CurrentMag = GetWorld()->SpawnActor<AMag>(OtherHand->EquippedWeapon->MagType, SpawnParameters);
		CurrentMag->AttachToComponent(HandMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "LeftMagSocket");
		CurrentMag->bEmpty = false;
	}
}

FName AVRHand::GetAppropriateSocket()
{
	if (OverlappingWeapon == nullptr) return FName();

	FName ReturnFName = FName("");
	switch (OverlappingWeapon->WeaponType)
	{
	case EWeaponType::EWT_Pistol:
		ReturnFName = FName("RightPistolSocket");
		break;
	case EWeaponType::EWT_AssaultRifle:
		ReturnFName = FName("RightARSocket");
		break;
	case EWeaponType::EWT_Shotgun:
		ReturnFName = FName("RightShotgunSocket");
		break;
	}
	return ReturnFName;
}

void AVRHand::DropWeapon()
{
	if (EquippedWeapon) // If we're actually holding a weapon this takes care of dropping it
	{
		EquippedWeapon->DetachAllSceneComponents(HandMesh, FDetachmentTransformRules::KeepWorldTransform);
		EquippedWeapon->Drop();
		if (OtherHand && OtherHand->GrippedWeapon == EquippedWeapon) // If the other hand is gripping the weapon when it's dropped this makes the grip stop
		{
			OtherHand->DropWeapon();
		}

		EquippedWeapon = nullptr;
		if (HandMeshAnimInstance)
		{
			HandMeshAnimInstance->bWeaponIsEquipped = false;
		}
	}
	else if (CurrentMag) // If we're holding a mag this handles dropping and getting rid of it
	{
		CurrentMag->DetachAllSceneComponents(HandMesh, FDetachmentTransformRules::KeepWorldTransform);
		CurrentMag->Drop();
		CurrentMag = nullptr;
	}
	else if (GrippedWeapon)
	{
		HandTarget = HandRoot;
		HandAttachment->AttachToComponent(HandRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		AShotgun* GrippedShotgun = Cast<AShotgun>(OverlappingWeapon);
		if (GrippedShotgun)
		{
			GrippedShotgun->GripMotionController = nullptr;
		}
		GrippedWeapon = nullptr;
		if (OtherHand)
		{
			OtherHand->bHandRotInterp = false;
			OtherHand->HandAttachment->SetWorldRotation(FRotator(0.f, 0.f, 0.f));
		}
	}
	else if (CurrentVault)
	{
		if (CurrentVault->HandsEquipped == 1)
		{
			OwnerCharacter->SetVaultMovementState(false);
			CurrentVault->SetPawnCollision(true);
			CurrentVault->PreviousFirstControllerPos = FVector(0.f, 0.f, 0.f);
		}
		if (CurrentVault->HandsEquipped == 2)
		{
			CurrentVault->PreviousSecondControllerPos = FVector(0.f, 0.f, 0.f);
		}
		HandTarget = HandRoot;
		HandAttachment->AttachToComponent(HandRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CurrentVault->HandsEquipped--;
		CurrentVault = nullptr;
	}
}

// cause they see right through me, they see right through me, they see through right through me, i see right through me, do you see right through me?

void AVRHand::FireWeaponPressed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->FirePressed();
	}
}

void AVRHand::FireWeaponReleased()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->FireReleased();
	}
}

void AVRHand::CheckMagInsert()
{
	if (OverlappingWeapon && !OverlappingWeapon->bHasMag && CurrentMag && !CurrentMag->bEmpty)
	{
		const float DistanceToReloadPoint = UKismetMathLibrary::Vector_DistanceSquared(HandRoot->GetComponentLocation(), OverlappingWeapon->ReloadPoint->GetComponentLocation());
		if (DistanceToReloadPoint < 60.f)
		{
			OverlappingWeapon->CurrentAmmo = OverlappingWeapon->MaxAmmo;
			CurrentMag->Destroy();
			if (CurrentMag->InsertSoundCue)
			{
				UGameplayStatics::PlaySoundAtLocation(this, CurrentMag->InsertSoundCue, CurrentMag->GetActorLocation());
			}
			CurrentMag = nullptr;
			OverlappingWeapon->bHasMag = true;
			OverlappingWeapon->PlayMagInAnimation();
		}
	}
}
