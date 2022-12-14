#pragma once

#include "CoreMinimal.h"
#include "BaseAbility.h"
#include "BaseAbilityClientToServer.generated.h"

UCLASS()
class PROJECTC_API UBaseAbilityClientToServer : public UBaseAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateServerAbility(const FGameplayAbilitySpecHandle Handle,
	                                   const FGameplayAbilityActorInfo* ActorInfo,
	                                   const FGameplayAbilityActivationInfo ActivationInfo,
	                                   const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbilityCleanup(const FGameplayAbilitySpecHandle Handle,
	                               const FGameplayAbilityActorInfo* ActorInfo,
	                               const FGameplayAbilityActivationInfo ActivationInfo,
	                               bool bReplicateEndAbility, bool bWasCancelled) override;
protected:
	UFUNCTION(BlueprintNativeEvent)
	void ActivateAbilityWithTargetData(const FGameplayAbilityTargetDataHandle& InData,
	                                   FGameplayTag ApplicationTag);

	virtual void ActivateAbilityWithTargetData_Implementation(const FGameplayAbilityTargetDataHandle& InData,
	                                                          FGameplayTag ApplicationTag)
	{
		unimplemented();
	}

	virtual void NotifyTargetDataReady(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

private:
	FDelegateHandle NotifyTargetDataReadyDelegateHandle;
};
