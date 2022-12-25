#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "ReloadAbility.generated.h"

UCLASS()
class PROJECTC_API UReloadAbility : public UBaseAbility
{
	GENERATED_BODY()

public:
	UReloadAbility();

	UFUNCTION(BlueprintCallable)
	float GetAmmo();

	UFUNCTION(BlueprintCallable)
	float GetMagazine();

	UFUNCTION(BlueprintCallable)
	void ReloadWeapon();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;

private:
	UPROPERTY(EditAnywhere)
	FGameplayAttribute AmmoAttribute;

	UPROPERTY(EditAnywhere)
	FGameplayAttribute MagazineAttribute;
	
	
};
