#include "Predicting/ZC/ZCLinearAI.h"
#include "Predicting/ZC/ZCLinearProjectile.h"
#include "Predicting/PredictingCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

#ifndef FAR_AWAY
#define FAR_AWAY 50'000
#endif // !FAR_AWAY


// Sets default values
AZCLinearAI::AZCLinearAI()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (ProximityTrigger == nullptr)
	{
		ProximityTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ProximityTrigger"));
		ProximityTrigger->bHiddenInGame = false; // shows the debug sphere in game which helps for testing
		SetRootComponent(ProximityTrigger);
	}

	if (VisualForm == nullptr)
	{
		VisualForm = CreateDefaultSubobject<USceneComponent>(TEXT("VisualForm"));
		if (VisualForm)
		{
			VisualForm->SetupAttachment(GetRootComponent());
		}
	}
}

// Called when the game starts or when spawned
void AZCLinearAI::BeginPlay()
{
	Super::BeginPlay();
	
	if (ProximityTrigger)
	{
		// Subscribe to overlap events
		if (!ProximityTrigger->OnComponentBeginOverlap.IsBound())
		{
			ProximityTrigger->OnComponentBeginOverlap.AddDynamic(this, &AZCLinearAI::OnBeginOverlap);
		}
		if (!ProximityTrigger->OnComponentEndOverlap.IsBound())
		{
			ProximityTrigger->OnComponentEndOverlap.AddDynamic(this, &AZCLinearAI::OnEndOverlap);
		}

		// Specify collision for overlap
		ProximityTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		ProximityTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AZCLinearAI::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		// Only track a new target if we don't already have one
		if (TargetChar == nullptr)
		{
			TargetChar = Cast<APredictingCharacter>(OtherActor);
			EnableAggro();
		}
	}
}

void AZCLinearAI::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		// If our target has left our trigger range, forget about them completely
		if (TargetChar && TargetChar->GetUniqueID() == OtherActor->GetUniqueID())
		{
			DisableAggro();
			TargetChar = nullptr;
		}
	}
}

// Called every frame
void AZCLinearAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FaceTarget(DeltaTime);
}

void AZCLinearAI::FaceTarget(float DeltaTime)
{
	if (TargetChar)
	{
		// Compare the forward look dir of the AI and the direction to the target.
		
		const FVector AIForwardDir = GetActorForwardVector(); // forward AI direction
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + AIForwardDir * FAR_AWAY, FColor:: Blue);
		
		const FVector DirToTarget = TargetChar->GetActorLocation() - GetActorLocation(); // direction towards target
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + DirToTarget * FAR_AWAY, FColor::Yellow);

		float ArcDistToTarget = FVector::DotProduct(AIForwardDir.GetSafeNormal(), DirToTarget.GetSafeNormal());
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(0, -1.f, FColor::Red, FString::Printf(TEXT("ArcDistToTarget: %f"), ArcDistToTarget));
		}
		float AngleDiffToTargetDeg = FMath::RadiansToDegrees(FMath::Acos(ArcDistToTarget));

		
		// Determine if the player is on the right or left so we know if we need to + or - the angle
		const FVector AIRightDir = GetActorRightVector();
		float AmountRightOrLeft = FVector::DotProduct(AIRightDir.GetSafeNormal(), DirToTarget.GetSafeNormal());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, -1.f, FColor::White, FString::Printf(TEXT("AmountRightOrLeft (- right), (+ left): %f"), AmountRightOrLeft));
		}


		// Rotate towards the target than many degrees.

		if (FMath::Abs(AmountRightOrLeft) > 0.01f) // Allow for _relatively_ close rotation, we don't need to be pinpoint exact (which actually causes issues with blipping back and forth if we try to get too exact)
		{
			FRotator CurrentRot = GetActorRotation();
			float NewYaw = CurrentRot.Yaw;
			if (AmountRightOrLeft < 0) // Target on right
			{
				NewYaw -= AngleDiffToTargetDeg;
			}
			else // Target on left
			{
				NewYaw += AngleDiffToTargetDeg;
			}

			// Lerp for smooth movement
			CurrentRot.Yaw = FMath::FInterpTo(CurrentRot.Yaw, NewYaw, DeltaTime, 10.f);
			// Set new posiiton
			SetActorRotation(CurrentRot);
		}
	}
}

/*
	Algorithm Summary:
		Based off (https://shellsphinx.github.io/images/PredictableProjectiles.pdf)

		Find the targets future position, at time (t), given:
			- projectile constant velocity
			- targets current instantaneous velocity (will account for average of history later) and position

		Equation for target's movement
			X'	= Targets future position
			Xot = Targets current position (origin)
			Vt	= Targets instantaneous velocity
			t	= time

			Linear equation:
				X' = Xot + (Vt * t)

		Equation for projectile's movement
			X'	= Projectiles future position
			Xop	= Projectiles current position (origin)
			Sp	= Projectiles constant speed
			t	= time

			Linear equation:
				X' = Xop + (Sp * t)  
			Or better the equation of a circle
				(X' - Xop)^2 = (Sp * t)^2

		Both equations have X' and t.
		So combine via X' and solve for t which gives us a quadratic that we can solve to get polynomial expansion: a^2 + b + c = 0
			
		a = (Vt^2 - Sp^2)
		b = (2 * (Xot - Xop) * Vt)
		c = (Xot - Xop)^2	

		then just solve the quadratic which gives (t) and plug back into Equation for target's movement to find X'

	Note: Any vector multiplies are just dot products
*/
bool AZCLinearAI::PredictTargetLocation(const float ProjectileSpeed, FVector& OutPredictedLocation)
{	
	if (TargetChar == nullptr)
	{
		return false;
	}

	const FVector Xot = TargetChar->GetActorLocation();	// Targets current position
	const FVector Vt = TargetChar->GetVelocity();		// Targets current velocity (speed * direction)

	const float Sp = ProjectileSpeed;					// Projectile constant speed
	const FVector Xop = GetActorLocation();				// Projectile current position (spawed at AI's position)

	// a = (Vt^2 - Sp^2)
	float a = FVector::DotProduct(Vt, Vt) - (Sp * Sp);

	const FVector XotDiffXop = Xot - Xop;

	// b = (2 * (Xow - Xop) * Vw)
	float b = 2 * FVector::DotProduct(XotDiffXop, Vt);

	// c = (Xow - Xop)^2
	float c = FVector::DotProduct(XotDiffXop, XotDiffXop);

	float t = 0.f;

	// Solve for Time (t) using quadratic
	if (a == 0.f)
	{// divide by 0 early out
		t = -(c / b);
	}
	else
	{
		// determinant: b^2 - 4ac
		float det = (b * b) - (4 * a * c);
		if (det < 0)
		{// invalid, just use target current position
			OutPredictedLocation = TargetChar->GetActorLocation();
			return true;
		}
		else if (det == 0)
		{// t = -b/2a
			t = -b / (2 * a);
		}
		else
		{
			// (-b + sqrt(b^2 - 4ac)) / 2a
			float tPlus = (-b + FMath::Sqrt((b * b) - (4 * a * c))) / (2 * a);

			// (-b - sqrt(b^2 - 4ac)) / 2a
			float tMinus = (-b - FMath::Sqrt((b * b) - (4 * a * c))) / (2 * a);

			if (tPlus > 0 && tMinus > 0)
			{
				t = FMath::Min(tPlus, tMinus);
			}
			else if (tPlus > 0)
			{
				t = tPlus;
			}
			else if (tMinus > 0)
			{
				t = tMinus;
			}
			else
			{
				ensureMsgf(false, TEXT("This should be mathematically impossible for both solutions (if we got this far) to be invalid"));
				return false;
			}
		}
	}

	// Now that we have Time (t) plug it back into Equation for target's movement
	OutPredictedLocation = Xot + (Vt * t);
	return true;
}

void AZCLinearAI::FireProjectile()
{
	if (GetWorld())
	{
		if (ProjectileClass)
		{
			AZCLinearProjectile* Proj = GetWorld()->SpawnActor<AZCLinearProjectile>(ProjectileClass);
			if (Proj)
			{
				// Spawns the projectile at our AIs position pointing in the same direction
				Proj->SetActorRotation(GetActorRotation());
				Proj->SetActorLocation(GetActorLocation());

				FVector PredictedTargetLoc;
				if (PredictTargetLocation(Proj->GetSpeed(), PredictedTargetLoc))
				{
					Proj->Init(PredictedTargetLoc);

					DrawDebugSphere(GetWorld(), PredictedTargetLoc, 15.f, 26, FColor::Red, false, 3.f);
					//DrawDebugBox(GetWorld(), FinalLocation, FVector(20.f, 20.f, 20.f), FQuat(), FColor::Green, true, 10.f);

					FVector DrawDir = PredictedTargetLoc - GetActorLocation();
					float TotalDist = DrawDir.Size();
					int NumBlocks = 20;
					float DistBetweenBlocks = TotalDist / NumBlocks;
					for (uint8 i = 0; i < NumBlocks; ++i)
					{
						const FVector ThisBlockLocation = GetActorLocation() + DrawDir.GetSafeNormal() * ((i + 1) * DistBetweenBlocks);
						DrawDebugSphere(GetWorld(), ThisBlockLocation, 8.f, 26, FColor::White, false, 3.f);
					}
				}
				else
				{// If we failed to find a target location just don't fire
					Proj->Destroy();
				}
			}
		}
	}
}

void AZCLinearAI::EnableAggro()
{
	if (GetWorldTimerManager().IsTimerActive(FireTimer))
	{
		GetWorldTimerManager().ClearTimer(FireTimer);
	}
	GetWorldTimerManager().SetTimer(FireTimer, this, &AZCLinearAI::FireProjectile, FireInterval, true);
}

void AZCLinearAI::DisableAggro()
{
	if (GetWorldTimerManager().IsTimerActive(FireTimer))
	{
		GetWorldTimerManager().ClearTimer(FireTimer);
	}
}

