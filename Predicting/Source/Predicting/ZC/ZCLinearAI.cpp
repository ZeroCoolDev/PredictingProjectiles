#include "Predicting/ZC/ZCLinearAI.h"
#include "Predicting/PredictingCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"

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
			TargetChar = nullptr;
		}

		UE_LOG(LogTemp, Warning, TEXT("Object [%d] has exited the trigger"), OtherActor->GetUniqueID());
	}
}

// Called every frame
void AZCLinearAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

