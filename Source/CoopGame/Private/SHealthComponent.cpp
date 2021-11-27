#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	DefaultHealth = 100;

	RegenerateHealthIncrement = 5;

	bIsDead = false;

	TeamNumber = 255;

	SetIsReplicated(true);
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();

		if(MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}	
	}
	Health = DefaultHealth;
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage <= 0.0f || bIsDead)
	{
		return;
	}

	if(IsFriendly(DamagedActor, DamageCauser))
	{
		UE_LOG(LogTemp, Log, TEXT("Same team so not applying damage."));

		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	OnHealthChanged.Broadcast(this, Health, Damage);

	bIsDead = Health <= 0.0f;

	if(bIsDead)
	{
		ASGameMode* GameMode = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if(GameMode)
		{
			GameMode->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
	
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Don't replicate back to owner as it has already played the fire animations.
	DOREPLIFETIME(USHealthComponent, Health);
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage);
}

void USHealthComponent::RegenerateHealth()
{
	if(Health == 0.0f)
	{
		// already dead!
		return;
	}

	Health = FMath::Clamp(Health + RegenerateHealthIncrement, 0.0f, DefaultHealth);

	OnHealthChanged.Broadcast(this, Health, -RegenerateHealthIncrement);
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if(ActorA == nullptr || ActorB == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Null actor so assuming friendly."));
		return true;
	}

	if(ActorA == ActorB)
	{
		// allow self damage
		UE_LOG(LogTemp, Log, TEXT("Same actor so returning not friendly."));
		return false;
	}

	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if(HealthCompA == nullptr || HealthCompB == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Null health comp so assuming friendly."));
		return true;
	}

	UE_LOG(LogTemp, Log, TEXT("Team A: %i Team B: %i"), HealthCompA->TeamNumber, HealthCompB->TeamNumber);

	return HealthCompA->TeamNumber == HealthCompB->TeamNumber;
}