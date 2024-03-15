// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mag.generated.h"

UCLASS()
class STARFALL_API AMag : public AActor
{
	GENERATED_BODY()
	
public:	
	AMag();
	virtual void Tick(float DeltaTime) override;

	void Drop();

	bool bEmpty = false;

	// sound cue for when mag is pulled out, public so hand can play the sound aka bad programming (:
	UPROPERTY(EditAnywhere, Category = SoundFX)
	class USoundCue* InsertSoundCue;

	UPROPERTY(EditAnywhere, Category = SoundFX)
	USoundCue* OutsertSoundCue; // this is definitely not a word, extract is prolly right word

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	USceneComponent* MagRootComponent;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MagMesh;

private:
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 1.f;

	void DestroyTimerFinished();

};
