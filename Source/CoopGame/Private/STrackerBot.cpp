#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "SHealthComponent.h"
#include "Engine/Public/DrawDebugHelpers.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	bUseVelocityChange = true;
	MovementForce = 500;
	RequiredDistanceToTarget = 100;

	ExplosionRadius = 200;
	ExplosionDamage = 40;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	NextPathPoint = GetNextPathPoint();
}

FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter *PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if(NavPath->PathPoints.Num() > 1)
	{
		return NavPath->PathPoints[1];
	}

	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct()
{
	if(bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Silver, false, 2.0f, 0, 1.0f);

	Destroy();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DistanceToPathPoint = (GetActorLocation() - NextPathPoint).Size();

	if(DistanceToPathPoint > RequiredDistanceToTarget)
	{
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();

		ForceDirection *= MovementForce;
		MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
	}
	else
	{
		NextPathPoint = GetNextPathPoint();
	}
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* SourceHealthComponent, float Health, float HealthDelta)
{
	if(MaterialInstance == nullptr)
	{
		MaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if(MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %f of %s"), Health, *GetName());

		// explode on death!
	if(Health <= 0.0f)
	{
		SelfDestruct();
	}

}