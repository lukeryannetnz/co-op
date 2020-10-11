#include "SProjectileWeapon.h"
#include "SProjectile.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"

void ASProjectileWeapon::Fire()
{
    AActor* MyOwner = GetOwner();
    if(!ProjectileClass || !MyOwner)
    {
        return;
    }

    //project projectile actor who explodes and causes damage to all around after 2 seconds
    FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
    FRotator MuzzleRotation = MeshComponent->GetSocketRotation(MuzzleSocketName);

    FVector EyeLocation;
    FRotator EyeRotation;

    MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

    // spawn the projectile at the muzzle.
    // deferred spawn allows us to set props on the object before it is spawned.
    ASProjectile* projectile = GetWorld()->SpawnActorDeferred<ASProjectile>(ProjectileClass, FTransform(EyeRotation, MuzzleLocation), this, UGameplayStatics::GetPlayerPawn(GetWorld(), 0), ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);

    if(projectile)
    {
        projectile->ExplosionEffect = this->ExplosionEffect;
        UGameplayStatics::FinishSpawningActor(projectile, FTransform(MuzzleRotation, MuzzleLocation));
    }
}