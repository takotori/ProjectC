#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "FireAbility.generated.h"


UCLASS()
class PROJECTC_API UFireAbility : public UBaseAbility
{
	GENERATED_BODY()

public:
	UFireAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							 const FGameplayAbilityActivationInfo ActivationInfo,
							 const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
};
