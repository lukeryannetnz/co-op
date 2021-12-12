#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "SHealthComponent.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Engine/Classes/Sound/SoundCue.h"

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

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));

	SphereComp->SetSphereRadius(500);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

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

	if(GetLocalRole() == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();

		AActor* Owner = GetOwner();
		if(Owner != NULL)
		{
			UWorld* World = Owner->GetWorld();
			if(World != NULL)
			{
				World->GetTimerManager().SetTimer(MemberTimerHandle, this, &ASTrackerBot::AdjustPowerLevel, 1.0f, true, 1.0f);
			}
		}
	}
}

void ASTrackerBot::AdjustPowerLevel()
{
	// find objects which are near
	TArray<FOverlapResult> Result;
	AActor* Owner = GetOwner();
	if(Owner != NULL)
	{
		UWorld* World = Owner->GetWorld();
		if(World != NULL)
		{
			FCollisionObjectQueryParams* ObjectQueryParams = new FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
			FCollisionShape CollisionShape;
			CollisionShape.SetSphere(600.0f);

			World->OverlapMultiByObjectType(Result, GetActorLocation(), FRotator(0.f, 0.f, 0.f).Quaternion(), *ObjectQueryParams, CollisionShape);
		}
	}

	// filter down to other ASTrackerBots
	int NearbyBotCount = 0;
	for(FOverlapResult Overlap : Result)
	{
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Overlap.GetActor());

		if(Bot && Bot != this)
		{
			if(NearbyBotCount <= MaxPowerLevel)
			{
				NearbyBotCount++;
			}
		}
	}	

	// update the power level
	PowerLevel = NearbyBotCount;

	UE_LOG(LogTemp, Log, TEXT("PowerLevel %f of %s"), PowerLevel, *GetName());

	// update the material variable "PowerLevelAlpha"
	if(MaterialInstance == nullptr)
	{
		MaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if(MaterialInstance)
	{
		// The material will glow red when PowerLevelAlpha is >0

		float alpha = PowerLevel / (float) MaxPowerLevel;
		UE_LOG(LogTemp, Log, TEXT("Setting PowerLevelAlpha of %f for %s"), PowerLevel, *GetName());
		MaterialInstance->SetScalarParameterValue("PowerLevelAlpha", alpha);
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter *PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if(NavPath && NavPath->PathPoints.Num() > 1)
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

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if(GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage + (PowerLevel * 2), GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		//DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Silver, false, 2.0f, 0, 1.0f);

		SetLifeSpan(2.0f);
	}

}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetLocalRole() == ROLE_Authority && !bExploded)
	{
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

	// explode on death!
	if(Health <= 0.0f)
	{
		SelfDestruct();
	}

}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if(bStartedSelfDestruction)
	{
		return;
	}

	ACharacter* PlayerPawn = Cast<ACharacter>(OtherActor);
	if(PlayerPawn)
	{
		if(GetLocalRole() == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, 0.5f, true, 0.0f);
		}

		bStartedSelfDestruction = true;

		UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}