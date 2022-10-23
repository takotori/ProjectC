// Fill out your copyright notice in the Description page of Project Settings.


#include "MannequinAnimInstance.h"

#include "MannequinCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMannequinAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	MannequinCharacter = Cast<AMannequinCharacter>(TryGetPawnOwner());
	
}

void UMannequinAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (MannequinCharacter == nullptr)
	{
		MannequinCharacter = Cast<AMannequinCharacter>(TryGetPawnOwner());
	}
	if (MannequinCharacter == nullptr) return;

	FVector Velocity = MannequinCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = MannequinCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MannequinCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bWeaponEquipped = true;
	bIsCrouched = MannequinCharacter->bIsCrouched;

	// Offset Yaw for Strafing
	FRotator AimRotation = MannequinCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MannequinCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MannequinCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	
}
