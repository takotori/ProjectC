#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectC/HUD/MannequinHUD.h"
#include "ProjectC/Types/CombatState.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTC_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	friend class AMannequinCharacter;
	void SpawnWeaponOnCharacter();
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void SingleShellReload();
	
	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

private:
	UPROPERTY()
	AMannequinCharacter* Character;

	UPROPERTY()
	class AMannequinPlayerController* Controller;

	UPROPERTY()
	class AMannequinHUD* HUD;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponToSpawn;

	UPROPERTY()
	AWeapon* EquippedWeapon;

	bool bFireButtonPressed;

	// HUD and crosshairs
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;

	FVector HitTarget;
	FHUDPackage HUDPackage;

	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();

	void FireTimerFinished();
	void Fire();

	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateSingleShellAmmoValue();
};
