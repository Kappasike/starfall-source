// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Shotgun.generated.h"

enum class EShotgunState : uint8
{
	ESS_CanFire,
	ESS_BoltStart,
	ESS_BoltEnd
};

UCLASS()
class STARFALL_API AShotgun : public AWeapon
{
	GENERATED_BODY()
public:
	AShotgun();
	virtual void Tick(float DeltaTime) override;

	virtual void FirePressed() override;
	virtual void FireReleased() override;

	void Fire();

	class UMotionControllerComponent* GripMotionController;

protected:
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* ShotgunMesh;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ShotgunBolt;

	UPROPERTY(EditAnywhere)
	USceneComponent* BoltEndPoint;

private:
	UPROPERTY(EditAnywhere)
	UAnimationAsset* FireAnimation;

	void TraceFromWeaponBarrel(FHitResult& OutHitResult);

	void UpdateBoltSlide();

	EShotgunState ShotgunState = EShotgunState::ESS_CanFire;

	float PreviousHandPositionX;

	float DeltaHandPositionX;

	UPROPERTY(EditAnywhere, Category = Sound)
	class USoundCue* InitialBoltSoundCue;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* FinalBoltSoundCue;

};
