#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../CoopGame.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVarDebugWeaponDrawing(
	TEXT("Coop.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw debug lines for weapons"),
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
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
	QueryParams.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	if(GetWorld()->LineTraceSingleByChannel(Hit, EyesLocation, LineTraceEnd, COLLISION_WEAPON, QueryParams))
	{
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), 20.0f, ShotDirection, Hit, Owner->GetInstigatorController(), this, DamageType);

		ApplyImpactEvent(Hit);

		TracerEndPoint = Hit.ImpactPoint;
	}

	ApplyMuzzleEffect();
	ApplyTracerEffect(TracerEndPoint);
	ApplyCameraShake();
	
	if(DebugWeaponDrawing > 0) {
		DrawDebugLine(GetWorld(), EyesLocation, LineTraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
	}
}

void ASWeapon::ApplyImpactEvent(FHitResult Hit)
{
	EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

	UParticleSystem* SelectedEffect = nullptr;

	switch (Surface)
	{
		case SURFACE_FLESHDEFAULT:
		case SURFACE_FLESHVULNERABLE:
			SelectedEffect = FleshImpactEffect;
			break;
		default:
			SelectedEffect = DefaultImpactEffect;
			break;
	}

	if(SelectedEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
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

void ASWeapon::ApplyCameraShake()
{
	APawn* MyOwner = Cast<APawn>(GetOwner());

	if(MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());

		if(PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}
