#include "FGameplayAbilityTargetData_STH.h"
#include "Serialization/Archive.h"

void FGameplayAbilityTargetData_STH::AddTargetDataToContext(FGameplayEffectContextHandle& Context,
	bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

	// Add game-specific data
	// if (FLyraGameplayEffectContext* TypedContext = FLyraGameplayEffectContext::ExtractEffectContext(Context))
	// {
	// 	TypedContext->CartridgeID = CartridgeID;
	// }
}

bool FGameplayAbilityTargetData_STH::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);
	Ar << CartridgeID;
	return true;
}
