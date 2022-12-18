
#include "CharacterInputConfig.h"

UCharacterInputConfig::UCharacterInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UCharacterInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FCharacterInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	return nullptr;
}

const UInputAction* UCharacterInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FCharacterInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	return nullptr;
}
