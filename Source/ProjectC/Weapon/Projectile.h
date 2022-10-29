#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class PROJECTC_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;


protected:
	virtual void BeginPlay() override;

public:	

};
