// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerUpInterval = 0.0f;
	PowerupDurationTicks = 0; 
	IsPowerupActive = false;

	SetReplicates(true);
}

void ASPowerupActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if(TicksProcessed >= PowerupDurationTicks)
	{
		OnExpired();
		
		IsPowerupActive = false;
		OnRep_PowerupActive();

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::ActivatePowerup(AActor* ActivateFor)
{
	OnActivated(ActivateFor);

	IsPowerupActive = true;
	OnRep_PowerupActive();

	if(PowerUpInterval > 0)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerUpInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupActiveChanged(IsPowerupActive);
}


void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, IsPowerupActive);
}