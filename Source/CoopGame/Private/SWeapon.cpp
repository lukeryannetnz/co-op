// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeapon::Fire()
{
	AActor* Owner = GetOwner();

	if(Owner)
	{
		FVector EyesLocation;
		FRotator EyesRotation;
		Owner->GetActorEyesViewPoint(EyesLocation, EyesRotation);
		FVector ShotDirection = EyesRotation.Vector();

		// trace from the eyes to a point in the distance, caluclated by taking the direction 
		// of gaze (rotation) and multipling it by a large constant
		FVector TraceEnd = EyesLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		FHitResult Hit;
		if(GetWorld()->LineTraceSingleByChannel(Hit, EyesLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			UGameplayStatics::ApplyPointDamage(Hit.GetActor(), 20.0f, ShotDirection, Hit, Owner->GetInstigatorController(), this, DamageType);
		}

		DrawDebugLine(GetWorld(), EyesLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
	}
}

