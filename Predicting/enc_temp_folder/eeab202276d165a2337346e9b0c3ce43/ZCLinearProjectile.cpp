#include "Predicting/ZC/ZCLinearProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
AZCLinearProjectile::AZCLinearProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (SceneComp == nullptr)
	{
		SceneComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ProjectileMesh"));
		SetRootComponent(SceneComp);
	}
}

void AZCLinearProjectile::Init(const FVector& StartLoc, const FVector& EndLoc)
{
	UE_LOG(LogTemp, Warning, TEXT("Start Projectile!"));
	SetActorLocation(StartLoc);
	ImpactPoint = EndLoc;
}

// Called when the game starts or when spawned
void AZCLinearProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AZCLinearProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move(DeltaTime);
	CheckForImpact();
}

void AZCLinearProjectile::Move(float DeltaTime)
{
	FVector MoveDir = ImpactPoint - GetActorLocation();
	FVector NewLoc = GetActorLocation() + (MoveDir.GetSafeNormal() * Speed * DeltaTime);
	SetActorLocation(NewLoc);
}

void AZCLinearProjectile::CheckForImpact()
{
	float DistFromImpact = (ImpactPoint - GetActorLocation()).Size();
	if (DistFromImpact <= KillDistance)
	{
		Destroy();
	}
}

