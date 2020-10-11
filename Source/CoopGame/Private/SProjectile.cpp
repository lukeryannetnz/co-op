#include "SProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/EngineTypes.h"

// Sets default values
ASProjectile::ASProjectile() : AActor()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp -> InitSphereRadius(5.0f);
    CollisionComp -> SetCollisionProfileName("Projectile");

	// Set as root component
    RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement -> UpdatedComponent = CollisionComp;
    ProjectileMovement -> InitialSpeed = 1000.0f;
    ProjectileMovement -> MaxSpeed = 1000.0f;
    ProjectileMovement -> bRotationFollowsVelocity = true;
    ProjectileMovement -> bShouldBounce = true;
}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	AActor* Owner = GetOwner();
	if(Owner != NULL)
	{
		UWorld* World = Owner->GetWorld();
		if(World != NULL)
		{
			World->GetTimerManager().SetTimer(MemberTimerHandle, this, &ASProjectile::Destruct, 1.0f, false, 1.0f);
		}
	}

	Super::BeginPlay();
}

void ASProjectile::Destruct()
{
	AActor* Owner = GetOwner(); 
	if(Owner != NULL)
	{
		UWorld* World = Owner->GetWorld();
		if(World != NULL)
		{
			World->GetTimerManager().ClearTimer(MemberTimerHandle);

			FVector location = CollisionComp->GetComponentLocation();
			if(ExplosionEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, ExplosionEffect, FTransform(location));
			}
			const TArray<AActor*> ignores;
			bool damageApplied = UGameplayStatics::ApplyRadialDamage(World, 20.0f, location, 250.0f, UDamageType::StaticClass(), ignores, this);

			if(damageApplied)
			{
				UE_LOG(LogTemp, Warning, TEXT("ASProjectile: Damage applied."));
			}
		}
	}

	Destroy(true, true);
}