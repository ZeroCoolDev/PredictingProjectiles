// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZCLinearAI.generated.h"

UCLASS()
class PREDICTING_API AZCLinearAI : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCLinearAI();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Triggers when the player enters the proximity
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Triggers when the player leaves the proximity
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	// Rotate AI so that it always faces the target
	void FaceTarget(float DeltaTime);

	// Given a constant speed, and a targets movement, determine how far to lead the target in order to hit it
	FVector PredictTargetLocation(const float ProjectileSpeed);

	// Fire projectiles mercilessly at the target if they are within range
	UFUNCTION()
	void FireProjectile();

	// Turn on the fire timer
	void EnableAggro();

	// Turn off the fire timer
	void DisableAggro();

	// Proximity trigger acting as our AI's eyes. Players within the trigger are now targets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* ProximityTrigger = nullptr;

	// Our visual representation in the world
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* VisualForm = nullptr;

	// The projectile type we spawn when firing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class TSubclassOf<class AZCLinearProjectile> ProjectileClass = nullptr;

	// For now this AI can only track whoever was the last player in the trigger
	class APredictingCharacter* TargetChar = nullptr;

	// Interval in seconds that we should fire a projectile
	float FireInterval = 1.f;
	FTimerHandle FireTimer;
};
