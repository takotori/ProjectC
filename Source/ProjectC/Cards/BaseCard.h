#pragma once

#include "CoreMinimal.h"
#include "BaseCard.generated.h"

UCLASS()
class PROJECTC_API ABaseCard : public AActor
{
	GENERATED_BODY()
	
public:
	ABaseCard();

protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MovementSpeedMultiplier = 1.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float DamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float BulletSpeed = 1.f;
	
private:
	
};
