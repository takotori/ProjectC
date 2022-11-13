#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class PROJECTC_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

	void SpawnPickup();
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
	void SpawnPickupTimerFinished();
	
private:
	FTimerHandle SpawnPickupTimer;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin = 15.f;
	
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax = 30.f;

};
