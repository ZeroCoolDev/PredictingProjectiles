#include "Predicting/ZC/ZCLinearAI.h"
#include "Predicting/ZC/ZCLinearProjectile.h"
#include "Predicting/PredictingCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
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
			UE_LOG(LogTemp, Warning, TEXT("Target updated to be [%d]"), TargetChar->GetUniqueID());
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

		UE_LOG(LogTemp, Warning, TEXT("Object [%d] has exited the trigger"), OtherActor->GetUniqueID());
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

FVector AZCLinearAI::PredictTargetLocation(const float ProjectileSpeed)
{	
	FVector EndLocation = FVector::ZeroVector;
	
	// For now just fire at the target
	if (TargetChar)
	{
		return TargetChar->GetActorLocation();
	}

	return EndLocation;
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
				FVector ExpectedTargetLoc = PredictTargetLocation(Proj->GetSpeed());
				Proj->Init(GetActorLocation(), ExpectedTargetLoc);
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
	GetWorldTimerManager().SetTimer(FireTimer, this, &AZCLinearAI::FireProjectile, FireInterval, false);
}

void AZCLinearAI::DisableAggro()
{
	if (GetWorldTimerManager().IsTimerActive(FireTimer))
	{
		GetWorldTimerManager().ClearTimer(FireTimer);
	}
}

