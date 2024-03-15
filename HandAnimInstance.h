// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HandAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class STARFALL_API UHandAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	bool BottomGrip;

	UPROPERTY(BlueprintReadOnly)
	bool IndexFinger;

	UPROPERTY(BlueprintReadOnly)
	bool Thumb;

	UPROPERTY(BlueprintReadOnly)
	bool bWeaponIsEquipped = false;
};
