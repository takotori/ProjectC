#pragma once

#include "CoreMinimal.h"
#include "BaseAbility.h"
#include "GameplayTagContainer.h"
#include "BaseAbilitySet.generated.h"

USTRUCT(BlueprintType)
struct FBaseAbilitySet_GameplayAbility 
{
	GENERATED_BODY()

public:
	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UBaseAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

UCLASS(BlueprintType, Const)
class UBaseAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UBaseAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	void GiveToAbilitySystem(UCardAbilitySystemComponent* CardASC, UObject* SourceObject) const;

protected:

	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FBaseAbilitySet_GameplayAbility> GrantedGameplayAbilities;
};
