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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	// Proximity trigger acting as our AI's eyes. Players within the trigger are now targets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* ProximityTrigger = nullptr;

	// Our visual representation in the world
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* VisualForm = nullptr;

	// The projectile type we spawn when firing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	class TSubclassOf<class AZCLinearProjectile> ProjectileClass = nullptr;
};
