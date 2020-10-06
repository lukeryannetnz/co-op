// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "SProjectile.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"

void ASProjectileWeapon::Fire()
{
    //project projectile actor who explodes and causes damage to all around after 2 seconds
    FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
    FRotator MuzzleRotation = MeshComponent->GetSocketRotation(MuzzleSocketName);

        // Set Spawn Collision Handling Override
    FActorSpawnParameters ActorSpawnParams;
    ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
    ActorSpawnParams.Instigator = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    ActorSpawnParams.Owner = this;

        // spawn the projectile at the muzzle
    GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
}