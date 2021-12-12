#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

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

	RemainingAmmunitionCount = 2500;

	SetReplicates(true); 

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	BulletSpreadDegrees = 1.0f;
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
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* Owner = GetOwner();

	if(!Owner)
	{
		return;
	}

	if(RemainingAmmunitionCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Out of ammo!"));
		return;
	}

	RemainingAmmunitionCount--;
	UE_LOG(LogTemp, Log, TEXT("Remaining ammo: %i"), RemainingAmmunitionCount);

	FVector EyesLocation;
	FRotator EyesRotation;
	Owner->GetActorEyesViewPoint(EyesLocation, EyesRotation);
	FVector ShotDirection = EyesRotation.Vector();

	// Spread bullets
	float HalfRad = FMath::DegreesToRadians(BulletSpreadDegrees);
	ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

	// trace from the eyes to a point in the distance, caluclated by taking the direction 
	// of gaze (rotation) and multipling it by a large constant
	FVector LineTraceEnd = EyesLocation + (ShotDirection * 10000);
	FVector TracerEndPoint = LineTraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	EPhysicalSurface Surface = SurfaceType_Default;
	FHitResult Hit;
	if(GetWorld()->LineTraceSingleByChannel(Hit, EyesLocation, LineTraceEnd, COLLISION_WEAPON, QueryParams))
	{
		Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		ActualDamage = BaseDamage;

		if(Surface == SURFACE_FLESHVULNERABLE)
		{
			ActualDamage *= 4.0f;
		}

		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage, ShotDirection, Hit, Owner->GetInstigatorController(), Owner, DamageType);
		UE_LOG(LogTemp, Log, TEXT("Damage applied: %f"), ActualDamage);
		ApplyImpactEvent(Hit.ImpactPoint, Surface);

		TracerEndPoint = Hit.ImpactPoint;
	}

	ApplyMuzzleEffect();
	ApplyTracerEffect(TracerEndPoint);
	ApplyCameraShake();
	
	if(DebugWeaponDrawing > 0) {
		DrawDebugLine(GetWorld(), EyesLocation, LineTraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
	}

	if(GetLocalRole() == ROLE_Authority)
	{
		HitScanTrace.TraceTo = TracerEndPoint;
		HitScanTrace.SurfaceType = 	Surface;
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::ApplyImpactEvent(FVector ImpactPoint, EPhysicalSurface Surface)
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
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
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

void ASWeapon::OnRep_HitScanTrace()
{
	ApplyMuzzleEffect();
	ApplyTracerEffect(HitScanTrace.TraceTo);
	ApplyImpactEvent(HitScanTrace.TraceTo, HitScanTrace.SurfaceType);
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Don't replicate back to owner as it has already played the fire animations.
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}