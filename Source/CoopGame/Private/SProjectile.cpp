// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ASProjectile::ASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp -> InitSphereRadius(5.0f);
    CollisionComp -> SetCollisionProfileName("Projectile");

	// Set as root component
    RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement -> UpdatedComponent = CollisionComp;
    ProjectileMovement -> InitialSpeed = 3000.0f;
    ProjectileMovement -> MaxSpeed = 3000.0f;
    ProjectileMovement -> bRotationFollowsVelocity = true;
    ProjectileMovement -> bShouldBounce = true;
}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// explode after 2 seconds
	// apply radial damage to all actors within a radius

}

