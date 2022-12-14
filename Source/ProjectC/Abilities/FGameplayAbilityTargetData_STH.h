#pragma once
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "FGameplayAbilityTargetData_STH.generated.h"

USTRUCT()
struct FGameplayAbilityTargetData_STH : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()
	FGameplayAbilityTargetData_STH() : CartridgeID(-1) {}

	virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

	/** ID to allow the identification of multiple bullets that were part of the same cartridge */
	UPROPERTY()
	int32 CartridgeID;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_STH> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_STH>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

