// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

class ASProjectile;

/**
 * 
 */
UCLASS()
class COOPGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()
	
protected:

	/** Projectile class to spawn */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    TSubclassOf<ASProjectile> ProjectileClass;

	virtual void Fire();
};
