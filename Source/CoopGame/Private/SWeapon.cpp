#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
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

	if(!Owner)
	{
		return;
	}

	FVector EyesLocation;
	FRotator EyesRotation;
	Owner->GetActorEyesViewPoint(EyesLocation, EyesRotation);
	FVector ShotDirection = EyesRotation.Vector();

	// trace from the eyes to a point in the distance, caluclated by taking the direction 
	// of gaze (rotation) and multipling it by a large constant
	FVector LineTraceEnd = EyesLocation + (ShotDirection * 10000);
	FVector TracerEndPoint = LineTraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	FHitResult Hit;
	if(GetWorld()->LineTraceSingleByChannel(Hit, EyesLocation, LineTraceEnd, ECC_Visibility, QueryParams))
	{
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), 20.0f, ShotDirection, Hit, Owner->GetInstigatorController(), this, DamageType);

		ApplyImpactEvent(Hit);

		TracerEndPoint = Hit.ImpactPoint;
	}

	ApplyMuzzleEffect();
	ApplyTracerEffect(TracerEndPoint);
	DrawDebugLine(GetWorld(), EyesLocation, LineTraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
}

void ASWeapon::ApplyImpactEvent(FHitResult Hit)
{
	if(ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
}

void ASWeapon::ApplyMuzzleEffect()
{
	if(MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}
}

void ASWeapon::ApplyTracerEffect(FVector TracerEndPoint)
{
	if(TracerEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if(TracerComponent)
		{
			TracerComponent->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
}
