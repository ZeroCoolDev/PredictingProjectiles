// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZCLinearProjectile.generated.h"

UCLASS()
class PREDICTING_API AZCLinearProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCLinearProjectile();

	void Init(const FVector& StartLoc, const FVector& EndLoc);

	FORCEINLINE float GetSpeed() { return Speed; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Move in a constant speed in a constant direction
	void Move(float DeltaTime);

	void CheckForImpact();

private:
	// How fast the projectile moves (constant speed)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	float Speed = 600.f;

	// Visual form of the projectile
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComp = nullptr;

	// Distance traveled before destroying itself
	float KillDistance = 50.f;
	
	// Intended impact point of then projectile
	FVector ImpactPoint;
};
