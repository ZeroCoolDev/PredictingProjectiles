#include "Predicting/ZC/ZCLinearProjectile.h"

#include "Components/SkeletalMeshComponent.h"

// Sets default values
AZCLinearProjectile::AZCLinearProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (ProjectileMesh == nullptr)
	{
		ProjectileMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ProjectileMesh"));
		SetRootComponent(ProjectileMesh);
	}
}

// Called when the game starts or when spawned
void AZCLinearProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Turn off all collision for the mesh
	ProjectileMesh->SetSimulatePhysics(false);
	ProjectileMesh->SetEnableGravity(false);
	ProjectileMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called every frame
void AZCLinearProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

