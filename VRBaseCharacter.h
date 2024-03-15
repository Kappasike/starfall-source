// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRBaseCharacter.generated.h"

UENUM(BlueprintType)
enum class EParkourMovementState : uint8
{
	EMS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EMS_WallRun UMETA(DisplayName = "WallRun"),
	EMS_WallRunDelay UMETA(DisplayName = "WallRunDelay"),
	EMS_Crouch UMETA(DisplayName = "Crouch"),
	EMS_Slide UMETA(DisplayName = "Slide"),
	EMS_Climb UMETA(DisplayName = "Climb"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class STARFALL_API AVRBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVRBaseCharacter();
	friend class AWeapon;
	friend class AVRHand;
	friend class AVault;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	void RightGripButtonPressed();
	void RightGripButtonReleased();

	void RightTriggerButtonPressed();
	void RightTriggerButtonReleased();

	void RightThumbButtonPressed();
	void RightThumbButtonReleased();

	void LeftGripButtonPressed();
	void LeftGripButtonReleased();
	
	void LeftTriggerButtonPressed();
	void LeftTriggerButtonReleased();

	void LeftThumbButtonPressed();
	void LeftThumbButtonReleased();

	void JumpButtonPressed();
	void JumpButtonReleased();

	void InitializeHands();
	
	void AlignCollisionHMD();
	void AlignCollisionHeight();
	void AlignHandsToTarget();

	UPROPERTY(EditAnywhere, Category = "VR")
	USceneComponent* VROrigin;

	UPROPERTY(EditAnywhere, Category = "VR")
	class UCameraComponent* VRCamera;

	UPROPERTY(EditAnywhere, Category = "VR")
	class UMotionControllerComponent* RightMotionController;

	UPROPERTY(EditAnywhere, Category = "VR")
	UMotionControllerComponent* LeftMotionController;

	/**
	* Wallrunning
	*/

	void WallRunUpdate();
	bool WallRunMovement(FHitResult& HitResult, FVector Start, FVector End, float WallRunDirection);
	void WallRunEnd();

	FTimerHandle WallRunDelayTimer;

	void WallRunDelayTimerFinished();

	FTimerHandle WallRunFallTimer;

	void WallRunFallTimerFinished();

	/**
	* Jumping
	*/

	void JumpModifierUpdate(float DeltaTime);

	/**
	* Sprinting
	*/

	FVector LeftHandLocationLastFrame;

	FVector RightHandLocationLastFrame;

	float LeftHandVelocity;
	
	float RightHandVelocity;

	void SprintCheckUpdate(float DeltaTime);

	FTimerHandle SprintCheckTimer;

	void SprintCheckTimerFinished();

	/**
	* Sliding and Crouching
	*/

	void CrouchSlideCheckUpdate(); // This just checks relative height of camera and if within a certain number it will crouch or slide based on velocity

	void CrouchInitiate();

	void CrouchFinish();

	void SlideInitiate();

	void SlideUpdate();

	void SlideFinish();

	FVector CalculateFloorInfluence(FVector FloorNormal);

	/**
	* Hands
	*/

	UPROPERTY(EditAnywhere, Category = Hands)
	TSubclassOf<class AVRHand> RightHandSpawnActor;
	AVRHand* RightHand;

	UPROPERTY(EditAnywhere, Category = Hands)
	TSubclassOf<AVRHand> LeftHandSpawnActor;
	AVRHand* LeftHand;

	/**
	* Ammo Pouch
	*/

	UPROPERTY(EditAnywhere, Category = "Ammo Pouch")
	class USphereComponent* PouchCollision;

	UPROPERTY(EditAnywhere, Category = "Ammo Pouch")
	UStaticMeshComponent* PouchMesh;

	UPROPERTY(EditAnywhere)
	USceneComponent* ActualLocation; // because capsule is root and half height is changed, this is a component which gives the actual world position of the actor

private:	
	
	/*class UHandAnimInstance* RightHandAnimInstance;

	UHandAnimInstance* LeftHandAnimInstance;

	USceneComponent* RightHandTarget;

	USceneComponent* LeftHandTarget;*/

	/**
	* General Movement
	*/

	float DeltaRotation;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	EParkourMovementState ParkourMovementState;

	/**
	* Wallrunning
	*/

	float WallRunSpeed = 1000.f;

	FVector WallRunNormal;

	bool bWallrunNoFall = true;

	/**
	* Jumping
	*/

	// This modifier is multiplied by the jump z velocity and
	// makes it so the longer you hold the button and the more
	// you jump in real life, the higher you jump
	float JumpModifier = 1.f;

	float ChargeJumpModifier = 1.f;
	bool bShouldIncreaseJumpModifier = false;

	// See the difference in height from when you press the jump button and when you release it
	float InitialCrouchJumpHeight;
	float DeltaCrouchJumpHeight;
	float CrouchJumpHeightMultiplier = 1.f;
	

	/**
	* Sprinting
	*/

	bool bDisableInput = false;

	/**
	* Weapons
	*/

	class AWeapon* BackHolsteredWeapon;

	AWeapon* SideHolsteredWeapon;

	/**
	* Vault
	*/

	void SetVaultMovementState(bool bGrabbing);

};
