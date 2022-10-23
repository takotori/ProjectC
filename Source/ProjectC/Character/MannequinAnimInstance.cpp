// Fill out your copyright notice in the Description page of Project Settings.


#include "MannequinAnimInstance.h"

#include "MannequinCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	
}
