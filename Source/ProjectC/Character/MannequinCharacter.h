#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectC/Interfaces/InteractWithCrosshairsInterface.h"
#include "MannequinCharacter.generated.h"

UCLASS()
class PROJECTC_API AMannequinCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	AMannequinCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void CrouchButtonPressed();
	void AimOffset(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();

private:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class AWeapon* Weapon;

	float AO_Yaw;
	float AO_Pitch;

	FRotator StartingAimRotation;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

public:
	AWeapon* GetEquippedWeapon();
	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FVector GetHitTarget() const;
};
