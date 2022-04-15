#include "Predicting/ZC/ZCLinearAI.h"

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
	
}

// Called every frame
void AZCLinearAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

