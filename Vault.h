// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vault.generated.h"

UCLASS()
class STARFALL_API AVault : public AActor
{
	GENERATED_BODY()
	
public:	
	AVault();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	USceneComponent* FirstGripLocation; // the z and y only matter, not x

	UPROPERTY(EditAnywhere)
	USceneComponent* SecondGripLocation; // the z and y only matter, not x

	UPROPERTY(EditAnywhere)
	USceneComponent* DefaultGripLocation; // bad naming but this is what values will be used for references because it's static

	uint8_t HandsEquipped = 0;

	UPROPERTY(EditAnywhere)
	FRotator InFrontRightHandRot;

	UPROPERTY(EditAnywhere)
	FRotator InFrontLeftHandRot;

	UPROPERTY(EditAnywhere)
	FRotator BehindRightHandRot;

	UPROPERTY(EditAnywhere)
	FRotator BehindLeftHandRot;

	UPROPERTY(EditAnywhere)
	FVector2D InFrontRightHandPos;

	UPROPERTY(EditAnywhere)
	FVector2D InFrontLeftHandPos;

	UPROPERTY(EditAnywhere)
	FVector2D BehindRightHandPos;

	UPROPERTY(EditAnywhere)
	FVector2D BehindLeftHandPos;

	USceneComponent* TargetPushComponent;

	class AVRBaseCharacter* OwnerCharacter;

	class UMotionControllerComponent* FirstMotionController;

	UMotionControllerComponent* SecondMotionController;

	FVector PreviousFirstControllerPos = FVector(0.f, 0.f, 0.f);

	FVector PreviousSecondControllerPos = FVector(0.f, 0.f, 0.f);

	void SetPawnCollision(bool bEnableCollision);

protected:
	UPROPERTY(EditAnywhere)
	USceneComponent* RootSceneComponent;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* VaultMesh;

	virtual void BeginPlay() override;

	void ClimbMovementUpdate();

private:
	

};
