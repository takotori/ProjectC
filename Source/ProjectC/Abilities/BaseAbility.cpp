#include "BaseAbility.h"

#include "AbilitySystemComponent.h"
#include "ProjectC/Weapon/Weapon.h"

UBaseAbility::UBaseAbility()
{
}

AMannequinCharacter* UBaseAbility::GetCharacterFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AMannequinCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

AController* UBaseAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (const APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}
	return nullptr;
}

UAbilitySystemComponent* UBaseAbility::GetAbilitySystemComponent() const
{
	return CurrentActorInfo ? Cast<UAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr;
}

AWeapon* UBaseAbility::GetWeaponInstance() const
{
	if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<AWeapon>(Spec->SourceObject.Get());
	}
	return nullptr;
}

void UBaseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	if (bHasBlueprintActivate)
	{
		K2_ActivateAbility();
		return;
	}
	if (bHasBlueprintActivateFromEvent && TriggerEventData)
	{
		K2_ActivateAbilityFromEvent(*TriggerEventData);
		return;
	}
	if (ActorInfo->IsLocallyControlled())
	{
		ActivateLocalPlayerAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
	else if (ActorInfo->IsNetAuthority())
	{
		ActivateServerAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
	else
	{
		// insufficient network role to activate ability
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
	}
}

void UBaseAbility::EndAbilityCleanup(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                     bool bWasCancelled)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
