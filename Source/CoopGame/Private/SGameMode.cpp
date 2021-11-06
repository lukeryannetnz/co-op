// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"


ASGameMode::ASGameMode()
{
    TimeBetweenWaves = 2.0f;   
}

void ASGameMode::StartWave()
{
    WaveCount++;

    NumberOfBotsToSpawn = 2 * WaveCount;

    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

}

void ASGameMode::SpawnBotTimerElapsed()
{
    SpawnNewBot();

    NumberOfBotsToSpawn--;

    if(NumberOfBotsToSpawn <= 0)
    {
        EndWave();
    }
}

void ASGameMode::StartPlay()
{
    Super::StartPlay();

    PrepareForNextWave();

}

void ASGameMode::EndWave()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

    PrepareForNextWave();
}

void ASGameMode::PrepareForNextWave()
{
    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::StartWave, 1.0f, true, 0.0f);
}
