#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "SprintAbility.generated.h"

UCLASS()
class PROJECTC_API USprintAbility : public UBaseAbility
{
	GENERATED_BODY()

public:
	USprintAbility();

protected:
	
	UFUNCTION(BlueprintCallable)
	void CharacterSprintStart();

	UFUNCTION(BlueprintCallable)
	void CharacterSprintStop();
};
