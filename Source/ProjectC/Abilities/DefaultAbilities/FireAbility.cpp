
#include "FireAbility.h"

#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Components/CombatComponent.h"

UFireAbility::UFireAbility()
{
	AbilityInputID = EGSAbilityInputID::Fire;
}

void UFireAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(ActorInfo->AvatarActor))
	{
		MannequinCharacter->Fire();
		if (UCombatComponent* CombatComponent = MannequinCharacter->GetCombat())
		{
			CombatComponent->FireButtonPressed(true);
		}
	}
}

void UFireAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	if (AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(ActorInfo->AvatarActor))
	{
		MannequinCharacter->FireButtonReleased();
		if (UCombatComponent* CombatComponent = MannequinCharacter->GetCombat())
		{
			CombatComponent->FireButtonPressed(false);
		}
	}
	
}
