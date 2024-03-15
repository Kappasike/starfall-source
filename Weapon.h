// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_NotWeapon UMETA(DisplayName = "NotWeapon"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubMachineGun UMETA(DisplayName = "SMG"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_UltraShotgun UMETA(DisplayName = "UltraShotgun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_SniperRifle UMETA(DisplayName = "SniperRifle"),

	EWT_Max UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class STARFALL_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	friend class AVRBaseCharacter;
	friend class AVRHand;
	virtual void Tick(float DeltaTime) override;

	virtual void FirePressed();
	virtual void FireReleased();

	virtual void Equip();
	virtual void Drop();

	UPROPERTY(EditAnywhere, Category = Stats)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	USceneComponent* ReloadPoint;

	int32 CurrentAmmo;

	bool bHasMag = true;

	virtual void PlayMagInAnimation();
	virtual void PlayMagOutAnimation();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* RootSceneComponent;

	UPROPERTY(EditAnywhere)
	USceneComponent* RootGripPoint;

	UPROPERTY(EditAnywhere)
	USceneComponent* LeftGripPoint;

	UPROPERTY(EditAnywhere)
	USceneComponent* RightGripPoint;

	UPROPERTY(EditAnywhere)
	USceneComponent* BarrelPoint;

	// When you go to reload this will be the type of mag that spawns in hand
	UPROPERTY(EditAnywhere, Category = Ammo)
	TSubclassOf<class AMag> MagType;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 MaxAmmo;

	UPROPERTY(EditAnywhere, Category = Ammo)
	UAnimationAsset* MagInLoop;

	UPROPERTY(EditAnywhere, Category = Ammo)
	UAnimationAsset* MagOutLoop;

	bool bIsAlreadyEquipped = false;

private:
	


};
