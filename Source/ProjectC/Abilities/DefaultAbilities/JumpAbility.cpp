
#include "JumpAbility.h"

UJumpAbility::UJumpAbility()
{
	AbilityInputID = EGSAbilityInputID::Jump;
}

void UJumpAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	CharacterJumpStop(); // In case ability blueprint doesn't call it
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UJumpAbility::CharacterJumpStart()
{
	if (AMannequinCharacter* MannequinCharacter = GetCharacterFromActorInfo())
	{
		if (MannequinCharacter->IsLocallyControlled() && !MannequinCharacter->bPressedJump)
		{
			MannequinCharacter->UnCrouch();
			MannequinCharacter->Jump();
		}
	}
}

void UJumpAbility::CharacterJumpStop()
{
	if (AMannequinCharacter* MannequinCharacter = GetCharacterFromActorInfo())
	{
		if (MannequinCharacter->IsLocallyControlled() && MannequinCharacter->bPressedJump)
		{
			MannequinCharacter->StopJumping();
		}
	}
}
