///
/// Game mode. Controls the game, only exists on the server - no instances on the clients so can't be replicated. 
/// For this reason it interacts with the replicated GameState class.
///

#include "SGameMode.h"
#include "SHealthComponent.h"

ASGameMode::ASGameMode()
{
    TimeBetweenWaves = 2.0f;

    GameStateClass = ASGameState::StaticClass();

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
}

void ASGameMode:: Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    CheckWaveState();
    CheckAnyPlayerAlive();
}

void ASGameMode::StartWave()
{
    WaveCount++;

    NumberOfBotsToSpawn = 2 * WaveCount;

    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

}

void ASGameMode::SpawnBotTimerElapsed()
{
    UE_LOG(LogTemp, Log, TEXT("Spawning a new bot."));

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

    UE_LOG(LogTemp, Log, TEXT("Ending wave"));

}

void ASGameMode::CheckWaveState()
{
    bool isPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

    if(isPreparingForWave || NumberOfBotsToSpawn > 0)
    {
        return;
    }

    bool isAnyBotAlive = false;

    for(FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
    {
        APawn* TestPawn = It->Get();

        if(TestPawn == nullptr || TestPawn->IsPlayerControlled())
        {
            continue;
        }

        USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

        if(HealthComp && HealthComp->GetHealth() > 0)
        {
            isAnyBotAlive = true;
            break;  
        }
    }

    if(!isAnyBotAlive)
    {
        PrepareForNextWave();
    }
}

void ASGameMode::CheckAnyPlayerAlive()
{
    for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
    {
        APlayerController* PC = It->Get();
        if(PC && PC->GetPawn())
        {
            APawn* MyPawn = PC->GetPawn();
            USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));

            if(HealthComp && HealthComp->GetHealth() > 0.0f)
            {
                // A player is alive
                return;
            }
        }
    }
    GameOver();
}

void ASGameMode::GameOver()
{
    EndWave();

    UE_LOG(LogTemp, Log, TEXT("GameOver!"));
}

void ASGameMode::PrepareForNextWave()
{
    UE_LOG(LogTemp, Log, TEXT("Preparing for next wave"));

    GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
    ASGameState* GameState = GetGameState<ASGameState>();

    if(GameState)
    {
        GameState->WaveState = NewState;
    }
}