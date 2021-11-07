// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SGameState.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"


enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor);

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	FTimerHandle TimerHandle_BotSpawner;
 
	FTimerHandle TimerHandle_NextWaveStart;

	int NumberOfBotsToSpawn;

	int WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void StartWave();

	void EndWave();

	void SpawnBotTimerElapsed();

	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

public:

	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;
};
