#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ProjectC/ProjectC.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "BaseAbility.generated.h"

/**
 * EBaseAbilityActivationPolicy
 *
 *	Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class ELyraAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned.
	OnSpawn
};

UCLASS()
class PROJECTC_API UBaseAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UBaseAbility();
	
	UFUNCTION(BlueprintCallable)
	AMannequinCharacter* GetCharacterFromActorInfo() const;

	UFUNCTION(BlueprintCallable)
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable)
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable)
	AWeapon* GetWeaponInstance() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "Activate Server Ability")
	void K2_ActivateServerAbility();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "Activate Local Player Ability")
	void K2_ActivateLocalPlayerAbility();

	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityInputID = EGSAbilityInputID::None;

	// Value to associate an ability with an slot without tying it to an automatically activated input.
	// Passive abilities won't be tied to an input so we need a way to generically associate abilities with slots.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityID = EGSAbilityInputID::None;

protected:
	virtual void ActivateLocalPlayerAbility(const FGameplayAbilitySpecHandle Handle,
	                                        const FGameplayAbilityActorInfo* ActorInfo,
	                                        const FGameplayAbilityActivationInfo ActivationInfo,
	                                        const FGameplayEventData* TriggerEventData)
	{
		K2_ActivateLocalPlayerAbility();
	}

	virtual void ActivateServerAbility(const FGameplayAbilitySpecHandle Handle,
	                                   const FGameplayAbilityActorInfo* ActorInfo,
	                                   const FGameplayAbilityActivationInfo ActivationInfo,
	                                   const FGameplayEventData* TriggerEventData) { K2_ActivateServerAbility(); }

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbilityCleanup(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                               const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                               bool bWasCancelled);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	ELyraAbilityActivationPolicy ActivationPolicy;

public:
	FORCEINLINE ELyraAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
};
