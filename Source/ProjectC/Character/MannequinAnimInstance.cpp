// Fill out your copyright notice in the Description page of Project Settings.


#include "MannequinAnimInstance.h"

#include "MannequinCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectC/Weapon/Weapon.h"

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
	EquippedWeapon = MannequinCharacter->GetEquippedWeapon();
	bIsCrouched = MannequinCharacter->bIsCrouched;
	bElimmed = MannequinCharacter->IsElimmed();

	// Offset Yaw for Strafing
	const FRotator AimRotation = MannequinCharacter->GetBaseAimRotation();
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MannequinCharacter->GetVelocity());
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MannequinCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = MannequinCharacter->GetAO_Yaw();
	AO_Pitch = MannequinCharacter->GetAO_Pitch();

	if (EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MannequinCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MannequinCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (MannequinCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MannequinCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);			
		}

	}
	bUseFABRIK = MannequinCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	if (MannequinCharacter->IsLocallyControlled() && MannequinCharacter->GetCombatState() == ECombatState::ECS_ThrowingGrenade)
	{
		bUseFABRIK = !MannequinCharacter->IsLocallyReloading();
	}
}
