
#include "DefaultAttributes.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UDefaultAttributes::UDefaultAttributes()
{
}

void UDefaultAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributes, Health, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributes, MaxHealth, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributes, Shield, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributes, MaxShield, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributes, MovementSpeed, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributes, WeaponDamage, COND_None, REPNOTIFY_Always)
}

void UDefaultAttributes::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributes, Health, OldHealth)
}

void UDefaultAttributes::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributes, MaxHealth, OldMaxHealth)
}

void UDefaultAttributes::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributes, Shield, OldShield)
}

void UDefaultAttributes::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributes, MaxShield, OldMaxShield)
}

void UDefaultAttributes::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributes, MovementSpeed, OldMovementSpeed)
}

void UDefaultAttributes::OnRep_WeaponDamage(const FGameplayAttributeData& OldWeaponDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributes, WeaponDamage, OldWeaponDamage)
}
