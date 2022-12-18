#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "JumpAbility.generated.h"

UCLASS()
class PROJECTC_API UJumpAbility : public UBaseAbility
{
	GENERATED_BODY()

public:
	UJumpAbility();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable)
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable)
	void CharacterJumpStop();
};
