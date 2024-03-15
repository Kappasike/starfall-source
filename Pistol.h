// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Pistol.generated.h"

/**
 * 
 */
UCLASS()
class STARFALL_API APistol : public AWeapon
{
	GENERATED_BODY()
public:
	APistol();

	virtual void FirePressed() override;
	virtual void FireReleased() override;

	virtual void PlayMagInAnimation() override;
	virtual void PlayMagOutAnimation() override;
	
protected:
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* PistolMesh;

	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	UAnimationAsset* FireAnimation;

	void TraceFromWeaponBarrel(FHitResult& OutHitResult);

	/**
	* When I make my own pistol model I'll change it so blueprints can read
	*/

	void Fire();

	bool bCanFire = true;

	FTimerHandle FireDelayTimer;

	UPROPERTY(EditAnywhere, Category = Ammo)
	float FireDelayTime;

	void FireDelayTimerFinished();

	bool bKeepFiring = false;

	UPROPERTY(EditAnywhere, Category = Ammo)
	bool bAutoFire = false;

};
