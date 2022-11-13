#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AMannequinCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	
private:
	UPROPERTY()
	class AMannequinCharacter* Character;

	bool bHealing = false;

	float HealingRate = 0.f;

	float AmountToHeal = 0.f;
};
