#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	bool bDied;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")	
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComponent;

	void MoveForward(float Value);

	void MoveRight (float Value);

	void BeginCrouch();

	void EndCrouch();

	void BeginJump();

	float DefaultFOV;

	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta=(ClampMin=0.1, ClampMax=100))
	float ZoomInterpSpeed;

	void BeginZoom();

	void EndZoom();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UFUNCTION()
	void HandleHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta);

private:

	bool bWantsToZoom;

};
