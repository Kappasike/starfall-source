// Fill out your copyright notice in the Description page of Project Settings.


#include "VRBaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../HandAnimInstance.h"
#include "../Items/Weapon.h"
#include "VRHand.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

// TODO: Refactor so hand is an actor and not all this garbage programming I have right now
//       Add shotgun class with pump action based on hand moving back and forth
//       Add sliding based on IRL crouching, should be easy
//       Add mantling rails, kong vault, fast vault, etc
//       If I'm feeling brave fix camera going out of map
//       Break free and leave us in ruins? Took this dagger in me and removed it? Gain the weight of you then lose it

AVRBaseCharacter::AVRBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(GetCapsuleComponent());

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(GetRootComponent());

	ActualLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ActualLocation"));
	ActualLocation->SetupAttachment(GetRootComponent());

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);

	/**
	* Left Hand
	*/

	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	LeftMotionController->SetupAttachment(VROrigin);
	LeftMotionController->SetTrackingSource(EControllerHand::Left);

	/**
	* Right Hand
	*/

	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	RightMotionController->SetupAttachment(VROrigin);
	RightMotionController->SetTrackingSource(EControllerHand::Right);

	/**
	* Ammo Pouch
	*/

	PouchCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PouchCollision"));
	PouchCollision->SetupAttachment(GetCapsuleComponent());

	PouchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PouchMesh"));
	PouchMesh->SetupAttachment(PouchCollision);

}

void AVRBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	InitializeHands();
}

void AVRBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AlignCollisionHMD();
	AlignCollisionHeight();
	WallRunUpdate();
	JumpModifierUpdate(DeltaTime);
	SprintCheckUpdate(DeltaTime);
	CrouchSlideCheckUpdate();
	SlideUpdate();
}

void AVRBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Move_X", this, &AVRBaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move_Y", this, &AVRBaseCharacter::MoveRight);

	/**
	* Right Hand
	*/

	PlayerInputComponent->BindAction("RightGripButton", IE_Pressed, this, &AVRBaseCharacter::RightGripButtonPressed);
	PlayerInputComponent->BindAction("RightGripButton", IE_Released, this, &AVRBaseCharacter::RightGripButtonReleased);

	PlayerInputComponent->BindAction("RightTriggerButton", IE_Pressed, this, &AVRBaseCharacter::RightTriggerButtonPressed);
	PlayerInputComponent->BindAction("RightTriggerButton", IE_Released, this, &AVRBaseCharacter::RightTriggerButtonReleased);

	PlayerInputComponent->BindAction("RightThumbButton", IE_Pressed, this, &AVRBaseCharacter::RightThumbButtonPressed);
	PlayerInputComponent->BindAction("RightThumbButton", IE_Released, this, &AVRBaseCharacter::RightThumbButtonReleased);

	/**
	* Left Hand
	*/

	PlayerInputComponent->BindAction("LeftGripButton", IE_Pressed, this, &AVRBaseCharacter::LeftGripButtonPressed);
	PlayerInputComponent->BindAction("LeftGripButton", IE_Released, this, &AVRBaseCharacter::LeftGripButtonReleased);

	PlayerInputComponent->BindAction("LeftTriggerButton", IE_Pressed, this, &AVRBaseCharacter::LeftTriggerButtonPressed);
	PlayerInputComponent->BindAction("LeftTriggerButton", IE_Released, this, &AVRBaseCharacter::LeftTriggerButtonReleased);

	PlayerInputComponent->BindAction("LeftThumbButton", IE_Pressed, this, &AVRBaseCharacter::LeftThumbButtonPressed);
	PlayerInputComponent->BindAction("LeftThumbButton", IE_Released, this, &AVRBaseCharacter::LeftThumbButtonReleased);

	/**
	* Actions
	*/

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AVRBaseCharacter::JumpButtonPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AVRBaseCharacter::JumpButtonReleased);
}

void AVRBaseCharacter::MoveForward(float Value)
{
	if ((VRCamera != nullptr) && (Value != 0.f) && !bDisableInput)
	{
		//const FRotator Rotation{ LeftMotionController->GetComponentRotation() };
		//const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FRotator Rotation = VRCamera->GetComponentRotation();
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AVRBaseCharacter::MoveRight(float Value)
{
	if ((VRCamera != nullptr) && (Value != 0.f))
	{
		//const FRotator Rotation{ LeftMotionController->GetComponentRotation() };
		//const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		if (bDisableInput)
		{
			Value = Value / 5.f;
		}
		const FRotator Rotation = VRCamera->GetComponentRotation();
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AVRBaseCharacter::LeftGripButtonPressed()
{
	if (LeftHand)
	{
		LeftHand->GripPressed();
	}
}

void AVRBaseCharacter::LeftGripButtonReleased()
{
	if (LeftHand)
	{
		LeftHand->GripReleased();
	}
}

void AVRBaseCharacter::LeftTriggerButtonPressed()
{
	if (LeftHand)
	{
		LeftHand->TriggerPressed();
	}
}

void AVRBaseCharacter::LeftTriggerButtonReleased()
{
	if (LeftHand)
	{
		LeftHand->TriggerReleased();
	}
}

void AVRBaseCharacter::LeftThumbButtonPressed()
{
	if (LeftHand)
	{
		LeftHand->ThumbPressed();
	}
}

void AVRBaseCharacter::LeftThumbButtonReleased()
{
	if (LeftHand)
	{
		LeftHand->ThumbReleased();
	}
}

void AVRBaseCharacter::RightGripButtonPressed()
{
	if (RightHand)
	{
		RightHand->GripPressed();
	}
}

void AVRBaseCharacter::RightGripButtonReleased()
{
	if (RightHand)
	{
		RightHand->GripReleased();
	}
}

void AVRBaseCharacter::RightTriggerButtonPressed()
{
	if (RightHand)
	{
		RightHand->TriggerPressed();
	}
}

void AVRBaseCharacter::RightTriggerButtonReleased()
{
	if (RightHand)
	{
		RightHand->TriggerReleased();
	}
}

void AVRBaseCharacter::RightThumbButtonPressed()
{
	if (RightHand)
	{
		RightHand->ThumbPressed();
	}
}

void AVRBaseCharacter::RightThumbButtonReleased()
{
	if (RightHand)
	{
		RightHand->ThumbReleased();
	}
}

/**
* Aligning Components with VR
*/

void AVRBaseCharacter::AlignCollisionHMD()
{
	FVector CameraLocation = VRCamera->GetComponentLocation();
	CameraLocation.Z = GetActorLocation().Z;
	FVector DeltaLocation = UKismetMathLibrary::Subtract_VectorVector(CameraLocation, GetActorLocation());
	GetCapsuleComponent()->AddWorldOffset(DeltaLocation);
	VROrigin->AddWorldOffset(UKismetMathLibrary::NegateVector(DeltaLocation));
}

void AVRBaseCharacter::AlignCollisionHeight()
{
	/*if (ParkourMovementState == EParkourMovementState::EMS_Climb) // if climbing we should be short, if not then dynamically change height of capsule to match camera height
	{
		GetCapsuleComponent()->SetCapsuleHalfHeight(22.f);
	}
	else
	{
		
		ActualLocation->SetWorldLocation(FVector(0.f, 0.f, GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		//float CameraHeight = VRCamera->GetComponentLocation().Z-VROrigin->GetComponentLocation().Z;
		float CameraHeight = VRCamera->GetComponentLocation().Z - ActualLocation->GetComponentLocation().Z;
		//GEngine->AddOnScreenDebugMessage(8, 0, FColor::Cyan, FString::Printf(TEXT("Test %f"), ActualLocation->GetComponentLocation().Z));
		if (CameraHeight < 20.f) return; //make sure to re-enable this
		float DeltaHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - (CameraHeight / 2.f);
		GetCapsuleComponent()->SetCapsuleHalfHeight(CameraHeight / 2.f);
		AddActorLocalOffset(FVector(0.f, 0.f, DeltaHeight));
		
		//GetCapsuleComponent()->SetCapsuleHalfHeight(88.f);
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, "Changing Height Dynamically");
	}*/

	ActualLocation->SetWorldLocation(FVector(0.f, 0.f, GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	//float CameraHeight = VRCamera->GetComponentLocation().Z-VROrigin->GetComponentLocation().Z;
	
	if (ParkourMovementState != EParkourMovementState::EMS_Climb)
	{
		float CameraHeight = VRCamera->GetComponentLocation().Z - ActualLocation->GetComponentLocation().Z;
		//GEngine->AddOnScreenDebugMessage(8, 0, FColor::Cyan, FString::Printf(TEXT("Test %f"), ActualLocation->GetComponentLocation().Z));
		if (CameraHeight < 20.f) return;
		float DeltaHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - (CameraHeight / 2.f);
		GetCapsuleComponent()->SetCapsuleHalfHeight(CameraHeight / 2.f);
		AddActorLocalOffset(FVector(0.f, 0.f, DeltaHeight));
	}
	else
	{
		float CameraHeight = VRCamera->GetComponentLocation().Z - ActualLocation->GetComponentLocation().Z-32.f;
		//GEngine->AddOnScreenDebugMessage(8, 0, FColor::Cyan, FString::Printf(TEXT("Test %f"), ActualLocation->GetComponentLocation().Z));
		if (CameraHeight < 20.f) return;
		float DeltaHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - ((CameraHeight) / 2.f);
		GetCapsuleComponent()->SetCapsuleHalfHeight((CameraHeight / 2.f));
		AddActorLocalOffset(FVector(0.f, 0.f, DeltaHeight));
	}
}

void AVRBaseCharacter::AlignHandsToTarget()
{
	/*if (RightHandTarget && LeftHandTarget)
	{
		//RightHand->SetWorldLocationAndRotation(RightHandTarget->GetComponentLocation(), RightHandTarget->GetComponentRotation());
		//LeftHand->SetWorldLocationAndRotation(LeftHandTarget->GetComponentLocation(), LeftHandTarget->GetComponentRotation());
		RightHand->SetWorldTransform(RightHandTarget->GetComponentTransform());
		LeftHand->SetWorldTransform(LeftHandTarget->GetComponentTransform());
	}*/
}

void AVRBaseCharacter::InitializeHands()
{
	FActorSpawnParameters SpawnParamameters;
	SpawnParamameters.Owner = this;
	LeftHand = GetWorld()->SpawnActor<AVRHand>(LeftHandSpawnActor, SpawnParamameters);
	RightHand = GetWorld()->SpawnActor<AVRHand>(RightHandSpawnActor, SpawnParamameters);
	if (LeftHand)
	{
		LeftHand->AttachToComponent(LeftMotionController, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	if (RightHand)
	{
		RightHand->AttachToComponent(RightMotionController, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	LeftHand->OtherHand = RightHand;
	LeftHand->ParentMotionController = LeftMotionController;
	LeftHand->OwnerCharacter = this;
	RightHand->OtherHand = LeftHand;
	RightHand->ParentMotionController = RightMotionController;
	RightHand->OwnerCharacter = this;
}

/**
* .Wallrunning Section
*/

void AVRBaseCharacter::WallRunUpdate()
{
	if ((ParkourMovementState != EParkourMovementState::EMS_Unoccupied && ParkourMovementState != EParkourMovementState::EMS_WallRun) || !GetCharacterMovement()->IsFalling() || GetVelocity().Size2D() < 800.f) return;
	const FVector RightEndPoint = (GetActorLocation()) + (VRCamera->GetRightVector() * 50.f) + (VRCamera->GetForwardVector() * -25.f);
	const FVector LeftEndPoint = (GetActorLocation()) + (VRCamera->GetRightVector() * -50.f) + (VRCamera->GetForwardVector() * -25.f);
	FHitResult HitResult;

	if (WallRunMovement(HitResult, GetActorLocation(), RightEndPoint, -1.f) || WallRunMovement(HitResult, GetActorLocation(), LeftEndPoint, 1.f))
	{
		if (ParkourMovementState == EParkourMovementState::EMS_Unoccupied)
		{
			WallRunSpeed = GetVelocity().Size2D();
			GetCharacterMovement()->GravityScale = 0.f;
			bWallrunNoFall = true;
			GetWorldTimerManager().SetTimer(WallRunFallTimer, this, &AVRBaseCharacter::WallRunFallTimerFinished, 2.f);
		}
		ParkourMovementState = EParkourMovementState::EMS_WallRun;
		GetCharacterMovement()->GravityScale = UKismetMathLibrary::FInterpTo(GetCharacterMovement()->GravityScale, 0.2f, GetWorld()->GetDeltaSeconds(), 2.f);
	}
	else if (ParkourMovementState == EParkourMovementState::EMS_WallRun)
	{
		WallRunEnd();
	}
}

bool AVRBaseCharacter::WallRunMovement(FHitResult& HitResult, FVector Start, FVector End, float WallRunDirection)
{
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
	if (HitResult.bBlockingHit)
	{
		bool bWallValid = UKismetMathLibrary::InRange_FloatFloat(HitResult.Normal.Z, -0.52f, 0.52f, false, false);
		if (bWallValid)
		{
			WallRunNormal = HitResult.Normal;

			LaunchCharacter((UKismetMathLibrary::Cross_VectorVector(WallRunNormal, FVector(0.f, 0.f, 1.f))) * (WallRunSpeed * WallRunDirection), true, bWallrunNoFall);
			return true;
		}
	}
	return false;
}

void AVRBaseCharacter::WallRunEnd()
{
	ParkourMovementState = EParkourMovementState::EMS_WallRunDelay;
	GetWorldTimerManager().SetTimer(WallRunDelayTimer, this, &AVRBaseCharacter::WallRunDelayTimerFinished, 0.35f);
	GetCharacterMovement()->GravityScale = 2.5f;
}

void AVRBaseCharacter::WallRunDelayTimerFinished()
{
	if (ParkourMovementState == EParkourMovementState::EMS_WallRunDelay)
	{
		ParkourMovementState = EParkourMovementState::EMS_Unoccupied;
	}
}

void AVRBaseCharacter::WallRunFallTimerFinished()
{
	bWallrunNoFall = false;
}

/**
* .Jumping Section
*/

void AVRBaseCharacter::JumpButtonPressed()
{
	if (ParkourMovementState == EParkourMovementState::EMS_WallRun)
	{
		WallRunEnd();
		LaunchCharacter(FVector(WallRunNormal.X * 900.f, WallRunNormal.Y * 900.f, 700.f), false, true);
		GetWorldTimerManager().SetTimer(WallRunDelayTimer, this, &AVRBaseCharacter::WallRunDelayTimerFinished, 0.35f);
	}
	else if (!GetCharacterMovement()->IsFalling())
	{
		bShouldIncreaseJumpModifier = true;
		InitialCrouchJumpHeight = VRCamera->GetComponentLocation().Z - VROrigin->GetComponentLocation().Z;
	}
}

void AVRBaseCharacter::JumpButtonReleased()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bShouldIncreaseJumpModifier = false;
		DeltaCrouchJumpHeight = VRCamera->GetComponentLocation().Z - VROrigin->GetComponentLocation().Z - InitialCrouchJumpHeight;
		//GEngine->AddOnScreenDebugMessage(1, 4, FColor::Cyan, FString::Printf(TEXT("CrouchDelta is %f"), DeltaCrouchJumpHeight));
		if (DeltaCrouchJumpHeight > 0.f)
		{
			CrouchJumpHeightMultiplier = DeltaCrouchJumpHeight / 100.f;
			CrouchJumpHeightMultiplier = 1.f + FMath::Clamp(CrouchJumpHeightMultiplier, 0.f, 0.5f);
		}
		JumpModifier = CrouchJumpHeightMultiplier * ChargeJumpModifier;
		LaunchCharacter(FVector(0.f, 0.f, 750.f * JumpModifier), false, false);
		JumpModifier = 1.f;
		ChargeJumpModifier = 1.f;
	}
}

void AVRBaseCharacter::JumpModifierUpdate(float DeltaTime)
{
	if (bShouldIncreaseJumpModifier)
	{
		ChargeJumpModifier += 0.5f * DeltaTime;
		ChargeJumpModifier = FMath::Clamp(ChargeJumpModifier, 1.f, 1.45f);

		const float CurrentCrouchJumpHeight = VRCamera->GetComponentLocation().Z - VROrigin->GetComponentLocation().Z;
		if (InitialCrouchJumpHeight > CurrentCrouchJumpHeight)
		{
			InitialCrouchJumpHeight = CurrentCrouchJumpHeight;
		}
		GEngine->AddOnScreenDebugMessage(1, 0, FColor::Magenta, FString::Printf(TEXT("JumpModifier is %f"), ChargeJumpModifier));
	}
}

/**
* .Sprinting Section
*/

void AVRBaseCharacter::SprintCheckUpdate(float DeltaTime)
{
	/**
	* Calculating velocity manually because motioncontroller component velocity is 0? 
	*/
	LeftHandVelocity = (LeftMotionController->GetRelativeLocation() - LeftHandLocationLastFrame).Size()/DeltaTime;
	RightHandVelocity = (RightMotionController->GetRelativeLocation() - RightHandLocationLastFrame).Size() / DeltaTime;

	LeftHandLocationLastFrame = LeftMotionController->GetRelativeLocation();
	RightHandLocationLastFrame = RightMotionController->GetRelativeLocation();
	//GEngine->AddOnScreenDebugMessage(3, 0, FColor::Blue, FString::Printf(TEXT("LeftMotionController Velocity: %f"), LeftHandVelocity));
	//GEngine->AddOnScreenDebugMessage(4, 0, FColor::Red, FString::Printf(TEXT("RightMotionController Velocity: %f"), RightHandVelocity));
	if ((LeftHandVelocity + RightHandVelocity) > 420.f) // zaza
	{
		GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Purple, "Sprint Activated");
		GetWorldTimerManager().SetTimer(SprintCheckTimer, this, &AVRBaseCharacter::SprintCheckTimerFinished, 1.25f);
		GetCharacterMovement()->MaxWalkSpeed = 1400.f;
	}
}

// I can't stop you putting roots in my dreamland, my house of stone, your ivy grows

void AVRBaseCharacter::SprintCheckTimerFinished()
{
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	if (ParkourMovementState == EParkourMovementState::EMS_WallRun)
	{
		WallRunEnd();
	}
}

/**
* .Sliding/Crouching Section
*/

void AVRBaseCharacter::CrouchSlideCheckUpdate()
{
	const float RealLifeHeight = VRCamera->GetComponentLocation().Z - VROrigin->GetComponentLocation().Z; // if less than a 100, initiate
	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("RealLifeHeight: %f"), RealLifeHeight));
	if (RealLifeHeight < 140.f && ParkourMovementState == EParkourMovementState::EMS_Unoccupied)
	{
		if (GetVelocity().Size() > 1000.f) // if velocity is greater than 1000 initiate slide, if not crouch
		{
			SlideInitiate();
			GEngine->AddOnScreenDebugMessage(0, 3, FColor::Red, "Slide Initiated");
		}
		else
		{
			CrouchInitiate();
			GEngine->AddOnScreenDebugMessage(1, 3, FColor::Blue, "Crouch Initiated");
		}
	}
	else if (RealLifeHeight > 140.f)
	{
		if (ParkourMovementState == EParkourMovementState::EMS_Slide)
		{
			SlideFinish();
		}
		else if (ParkourMovementState == EParkourMovementState::EMS_Crouch)
		{
			CrouchFinish();
		}
	}
}

void AVRBaseCharacter::CrouchInitiate()
{
	ParkourMovementState = EParkourMovementState::EMS_Crouch;
	GetCharacterMovement()->MaxWalkSpeed = 350.f;
}

void AVRBaseCharacter::CrouchFinish()
{
	ParkourMovementState = EParkourMovementState::EMS_Unoccupied;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void AVRBaseCharacter::SlideInitiate()
{
	ParkourMovementState = EParkourMovementState::EMS_Slide;
	bDisableInput = true;
	GetCharacterMovement()->GroundFriction = 0.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1024.f;
	GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity * 1.5f;
}

void AVRBaseCharacter::SlideUpdate()
{
	if (ParkourMovementState != EParkourMovementState::EMS_Slide) return;
	FHitResult Floor = GetCharacterMovement()->CurrentFloor.HitResult;
	//GEngine->AddOnScreenDebugMessage(32, 0, FColor::Emerald, FString::Printf(TEXT("Floor Normal: %s"), *CalculateFloorInfluence(Floor.Normal).ToString()));
	//GEngine->AddOnScreenDebugMessage(33, 0, FColor::Blue, FString::Printf(TEXT("Floor Influence: %s"), *Floor.Normal.ToString()));
	GetCharacterMovement()->AddForce(CalculateFloorInfluence(Floor.Normal) * 200000.f);
	if (GetVelocity().Size() <= 30.f)
	{
		SlideFinish();
	}
}

void AVRBaseCharacter::SlideFinish()
{
	ParkourMovementState = EParkourMovementState::EMS_Unoccupied;
	bDisableInput = false;
	GetCharacterMovement()->GroundFriction = 8.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
}

FVector AVRBaseCharacter::CalculateFloorInfluence(FVector FloorNormal)
{
	if (UKismetMathLibrary::EqualEqual_VectorVector(FloorNormal, FVector(0.f, 0.f, 1.f)))
	{
		return FVector(0.f, 0.f, 0.f);
	}
	FVector ReturnVector = UKismetMathLibrary::Cross_VectorVector(FloorNormal, UKismetMathLibrary::Cross_VectorVector(FloorNormal, FVector(0.f, 0.f, 1.f)));
	UKismetMathLibrary::Vector_Normalize(ReturnVector);
	return ReturnVector;
}

void AVRBaseCharacter::SetVaultMovementState(bool bGrabbing)
{
	if (bGrabbing)
	{
		ParkourMovementState = EParkourMovementState::EMS_Climb;
		GetCharacterMovement()->GravityScale = 0.f;
		//GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	else
	{
		ParkourMovementState = EParkourMovementState::EMS_Unoccupied;
		GetCharacterMovement()->GravityScale = 2.5f;
		//GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
		//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	}
}
