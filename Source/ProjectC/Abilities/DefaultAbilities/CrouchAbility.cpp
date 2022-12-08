
#include "CrouchAbility.h"

UCrouchAbility::UCrouchAbility()
{
	AbilityInputID = EGSAbilityInputID::Crouch;
}

void UCrouchAbility::CharacterCrouchStart()
{
	if (AMannequinCharacter* MannequinCharacter = GetCharacterFromActorInfo())
	{
		if (MannequinCharacter->IsLocallyControlled())
		{
			MannequinCharacter->Crouch();
		}
	}
}

void UCrouchAbility::CharacterCrouchStop()
{
	if (AMannequinCharacter* MannequinCharacter = GetCharacterFromActorInfo())
	{
		if (MannequinCharacter->IsLocallyControlled())
		{
			MannequinCharacter->UnCrouch();
		}
	}
}
