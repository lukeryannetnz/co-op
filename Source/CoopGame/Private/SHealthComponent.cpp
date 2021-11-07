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

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	OnHealthChanged.Broadcast(this, Health, Damage);

	bIsDead = Health <= 0.0f;

	if(bIsDead)
	{
		ASGameMode* GameMode = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if(GameMode)
		{
			GameMode->OnActorKilled.Broadcast(DamagedActor, DamageCauser);
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