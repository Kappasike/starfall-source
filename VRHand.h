// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRHand.generated.h"

UCLASS()
class STARFALL_API AVRHand : public AActor
{
	GENERATED_BODY()
	
public:	
	AVRHand();
	friend class AVRBaseCharacter;
	friend class AWeapon;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = Hand)
	USceneComponent* HandRoot;

	UPROPERTY(EditAnywhere, Category = Hand)
	USceneComponent* HandAttachment;

	UPROPERTY(EditAnywhere, Category = Hand)
	USkeletalMeshComponent* HandMesh;

	UPROPERTY(EditAnywhere, Category = Hand)
	class USphereComponent* HandCollision;

	UFUNCTION()
	void OnHandCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHandCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void AlignHandWithTarget();

	/**
	* Hand Inputs
	*/

	void GripPressed();
	void GripReleased();

	void TriggerPressed();
	void TriggerReleased();

	void ThumbPressed();
	void ThumbReleased();

	// Will have a separate function for regular items when I stop being lazy and add them
	void GrabWeapon();

	void DropWeapon();

	void FireWeaponPressed();

	void FireWeaponReleased();

	void CheckMagInsert();

private:
	/**
	* Weapon
	*/
	class AWeapon* OverlappingWeapon;

	AWeapon* EquippedWeapon;

	AWeapon* GrippedWeapon;

	class UHandAnimInstance* HandMeshAnimInstance;

	USceneComponent* HandTarget;

	// this is ugly but cheapest solution, enum would look better but waste of lines of code
	UPROPERTY(EditAnywhere)
	bool bIsRightHand;

	AVRHand* OtherHand;

	class AMag* CurrentMag;

	bool bCanGrabAmmo = false;

	FName GetAppropriateSocket();

	class UMotionControllerComponent* ParentMotionController;

	/**
	* Vaulting
	*/

	class AVault* OverlappingVault;

	AVault* CurrentVault;

	/**
	* Owner Character
	*/

	class AVRBaseCharacter* OwnerCharacter;

public:
	
	bool bHandRotInterp = false;

	FRotator InitialHandRotation;

};