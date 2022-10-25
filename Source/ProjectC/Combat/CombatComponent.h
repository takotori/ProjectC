// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


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

private:
	AMannequinCharacter* Character;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponToSpawn;

	AWeapon* EquippedWeapon;

public:	
		
};
