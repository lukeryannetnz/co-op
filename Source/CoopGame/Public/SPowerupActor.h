// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Time between powerup ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerUpInterval;

	/* The number of ticks for which the power up is active */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 PowerupDurationTicks;

	FTimerHandle TimerHandle_PowerupTick;

	UFUNCTION()
	void OnTickPowerup();

	int TicksProcessed;

public:	

	UFUNCTION()
	void ActivatePowerup();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

};
