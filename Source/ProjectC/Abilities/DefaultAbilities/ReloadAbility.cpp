#include "ReloadAbility.h"

#include "AbilitySystemComponent.h"

UReloadAbility::UReloadAbility()
{
}

float UReloadAbility::GetAmmo()
{
	if (const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(); AmmoAttribute.IsValid())
	{
		return AbilitySystemComponent->GetNumericAttribute(AmmoAttribute);
	}
	return -1.f;
}

float UReloadAbility::GetMagazine()
{
	if (const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(); MagazineAttribute.IsValid())
	{
		return AbilitySystemComponent->GetNumericAttribute(MagazineAttribute);
	}
	return -1.f;
}

void UReloadAbility::ReloadWeapon()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
	{
		AbilitySystemComponent->SetNumericAttributeBase(AmmoAttribute, GetMagazine());
	}
}

bool UReloadAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayTagContainer* SourceTags,
                                        const FGameplayTagContainer* TargetTags,
                                        FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(); AmmoAttribute.IsValid() &&
		MagazineAttribute.IsValid())
	{
		const float Ammo = AbilitySystemComponent->GetNumericAttribute(AmmoAttribute);
		const float Magazine = AbilitySystemComponent->GetNumericAttribute(MagazineAttribute);
		if (Ammo == Magazine)
		{
			return false;
		}
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}
