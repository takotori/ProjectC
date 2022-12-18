
#include "BaseAbilitySet.h"

#include "ProjectC/Components/CardAbilitySystemComponent.h"

UBaseAbilitySet::UBaseAbilitySet(const FObjectInitializer& ObjectInitializer)
{
}

void UBaseAbilitySet::GiveToAbilitySystem(UCardAbilitySystemComponent* CardASC, UObject* SourceObject) const
{
	check(CardASC);
	
	if (!CardASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FBaseAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			continue;
		}

		UBaseAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UBaseAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);
		AbilitySpec.SourceObject = SourceObject;

		const FGameplayAbilitySpecHandle AbilitySpecHandle = CardASC->GiveAbility(AbilitySpec);
	}

}
