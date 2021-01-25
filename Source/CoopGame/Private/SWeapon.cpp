#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../CoopGame.h"
#include "TimerManager.h"

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

	BaseDamage = 20.0f;
	RateOfFire = 600;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->GetTimeSeconds(), 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true), FirstDelay;

	Fire();
}
	
void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
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
		EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		ActualDamage = BaseDamage;

		if(Surface == SURFACE_FLESHVULNERABLE)
		{
			ActualDamage *= 4.0f;
		}

		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage, ShotDirection, Hit, Owner->GetInstigatorController(), this, DamageType);
		UE_LOG(LogTemp, Warning, TEXT("Damage applied: %f"), ActualDamage);
		ApplyImpactEvent(Hit, Surface);

		TracerEndPoint = Hit.ImpactPoint;
	}

	ApplyMuzzleEffect();
	ApplyTracerEffect(TracerEndPoint);
	ApplyCameraShake();
	
	if(DebugWeaponDrawing > 0) {
		DrawDebugLine(GetWorld(), EyesLocation, LineTraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void ASWeapon::ApplyImpactEvent(FHitResult Hit, EPhysicalSurface Surface)
{
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
