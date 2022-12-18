#include "JumpAbility.h"

UJumpAbility::UJumpAbility()
{
	AbilityInputID = EGSAbilityInputID::Jump;
}

bool UJumpAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
  	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || !Cast<AMannequinCharacter>(ActorInfo->AvatarActor.Get()))
	{
		return false;
	}
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	return true;
}

void UJumpAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                              bool bWasCancelled)
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
			if (MannequinCharacter->bIsCrouched)
			{
				MannequinCharacter->UnCrouch();
			}
			else
			{
				MannequinCharacter->Jump();
			}
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
