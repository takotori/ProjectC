
#include "WeaponAttributes.h"
#include "Net/UnrealNetwork.h"

UWeaponAttributes::UWeaponAttributes()
{
}

void UWeaponAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributes, Damage, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributes, RoundsPerMinute, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributes, Magazine, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributes, BulletSpeed, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributes, Spread, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributes, bCanBounce, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributes, bCanExplode, COND_None, REPNOTIFY_Always)
}

void UWeaponAttributes::OnRep_Damage(const FGameplayAttributeData& OldDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributes, Damage, OldDamage)
}

void UWeaponAttributes::OnRep_RoundPerMinute(const FGameplayAttributeData& OldRoundsPerMinute)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributes, RoundsPerMinute, OldRoundsPerMinute)
}

void UWeaponAttributes::OnRep_Magazine(const FGameplayAttributeData& OldMagazine)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributes, Magazine, OldMagazine)
}

void UWeaponAttributes::OnRep_BulletSpeed(const FGameplayAttributeData& OldVelocity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributes, BulletSpeed, OldVelocity)
}

void UWeaponAttributes::OnRep_Spread(const FGameplayAttributeData& OldSpread)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributes, Spread, OldSpread)
}

void UWeaponAttributes::OnRep_CanBounce(const FGameplayAttributeData& OldCanBounce)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributes, bCanBounce, OldCanBounce)
}

void UWeaponAttributes::OnRep_CanExplode(const FGameplayAttributeData& OldCanExplode)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributes, bCanExplode, OldCanExplode)
}
