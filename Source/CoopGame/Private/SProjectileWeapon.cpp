#include "SProjectileWeapon.h"
#include "SProjectile.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"

void ASProjectileWeapon::Fire()
{
    //project projectile actor who explodes and causes damage to all around after 2 seconds
    FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
    FRotator MuzzleRotation = MeshComponent->GetSocketRotation(MuzzleSocketName);

    // spawn the projectile at the muzzle.
    // deferred spawn allows us to set props on the object before it is spawned.
    ASProjectile* projectile = GetWorld()->SpawnActorDeferred<ASProjectile>(ProjectileClass, FTransform(MuzzleRotation, MuzzleLocation), this, UGameplayStatics::GetPlayerPawn(GetWorld(), 0), ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);

    if(projectile)
    {
        projectile->ExplosionEffect = this->ExplosionEffect;
        UGameplayStatics::FinishSpawningActor(projectile, FTransform(MuzzleRotation, MuzzleLocation));
    }
}