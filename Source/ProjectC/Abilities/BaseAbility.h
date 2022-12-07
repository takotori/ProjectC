#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ProjectC/ProjectC.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "BaseAbility.generated.h"

UCLASS()
class PROJECTC_API UBaseAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	AMannequinCharacter* GetCharacterFromActorInfo() const;
	
	
	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityInputID = EGSAbilityInputID::None;

	// Value to associate an ability with an slot without tying it to an automatically activated input.
	// Passive abilities won't be tied to an input so we need a way to generically associate abilities with slots.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityID = EGSAbilityInputID::None;
};
