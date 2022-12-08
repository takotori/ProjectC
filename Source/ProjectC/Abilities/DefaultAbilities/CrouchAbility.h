#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "CrouchAbility.generated.h"

UCLASS()
class PROJECTC_API UCrouchAbility : public UBaseAbility
{
	GENERATED_BODY()

public:
	UCrouchAbility();

protected:
	UFUNCTION(BlueprintCallable)
	void CharacterCrouchStart();

	UFUNCTION(BlueprintCallable)
	void CharacterCrouchStop();
};
