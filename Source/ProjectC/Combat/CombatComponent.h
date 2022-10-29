#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


#define TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTC_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class AMannequinCharacter;
	void SpawnWeaponOnCharacter();

protected:
	virtual void BeginPlay() override;
	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

private:
	AMannequinCharacter* Character;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponToSpawn;

	AWeapon* EquippedWeapon;

	bool bFireButtonpressed;
};
