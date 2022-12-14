#pragma once

#include "CoreMinimal.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "ProjectC/Abilities/BaseAbilityClientToServer.h"
#include "FireAbility.generated.h"

UCLASS()
class PROJECTC_API UFireAbility : public UBaseAbilityClientToServer
{
	GENERATED_BODY()

public:
	UFireAbility();

protected:
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

		FRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			  , EndAim(ForceInitToZero)
			  , AimDir(ForceInitToZero)
		{
		}
	};

	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	UFUNCTION(BlueprintCallable)
	void UseAbility(const FGameplayAbilityTargetDataHandle& TargetDataHandle,FGameplayTag ApplicationTag);

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
};
