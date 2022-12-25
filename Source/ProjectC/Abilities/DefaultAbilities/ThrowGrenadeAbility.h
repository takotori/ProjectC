#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "ThrowGrenadeAbility.generated.h"

UCLASS()
class PROJECTC_API UThrowGrenadeAbility : public UBaseAbility
{
	GENERATED_BODY()

public:
	UThrowGrenadeAbility();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;
};
