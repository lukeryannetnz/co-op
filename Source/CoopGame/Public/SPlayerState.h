/*
	Player Controllers only exist on the local machine and the server. They are not replicated. To replicate
	player scores to all machines we need to use Player State.
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public: 
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddScore(float Delta);
};
