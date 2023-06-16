#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "ProjectC/Abilities/BaseAbilityClientToServer.h"
#include "FireAbility.generated.h"

struct FRangedWeaponFiringInput
{
	// Start of the trace
	FVector StartTrace;

	// End of the trace if aim were perfect
	FVector EndAim;

	// The direction of th etrace if aim were perfect
	FVector AimDir;

	// The weapon instance / source of weapon data
	// ULyraRangedWeaponInstance* WeaponData = nullptr;

	// Can we play bullet FX for hits during this trace
	// bool bCanPlayBulletFX = false;

	FRangedWeaponFiringInput() : StartTrace(ForceInitToZero), EndAim(ForceInitToZero), AimDir(ForceInitToZero)
	{
	}
};

USTRUCT(BlueprintType)
struct FProjectileSpawnTransform
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector SocketTransform;

	UPROPERTY(BlueprintReadWrite)
	FRotator TargetRotation;

	UPROPERTY(BlueprintReadWrite)
	APawn* InstigatorPawn;
};

UCLASS()
class PROJECTC_API UFireAbility : public UBaseAbilityClientToServer
{
	GENERATED_BODY()

public:
	UFireAbility();

protected:
	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	UFUNCTION(BlueprintCallable)
	FProjectileSpawnTransform UseAbility(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);

	UFUNCTION(BlueprintCallable)
	bool HasEnoughAmmo();

	void PerformLocalTargeting(TArray<FHitResult>& OutHits);
	FVector GetWeaponTargetingSourceLocation();
	FTransform GetTargetingTransform(const APawn* SourcePawn);

	void TraceBullets(const FRangedWeaponFiringInput& InputData, TArray<FHitResult>& OutHits);
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace,
	                       OUT TArray<FHitResult>& OutHitResults) const;
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace,
	                               OUT TArray<FHitResult>& OutHits) const;
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);
	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	FProjectileSpawnTransform GetProjectileSpawnTransform(const FHitResult* HitResult, const FGameplayAbilityTargetDataHandle& TargetDataHandle);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	FGameplayAttribute AmmoAttribute;
};
