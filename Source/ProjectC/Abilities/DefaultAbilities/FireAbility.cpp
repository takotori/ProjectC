#include "FireAbility.h"

#include <map>

#include "AbilitySystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ProjectC/Abilities/FGameplayAbilityTargetData_STH.h"
#include "ProjectC/Components/CombatComponent.h"
#include "ProjectC/Weapon/Weapon.h"

UFireAbility::UFireAbility()
{
}

void UFireAbility::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	const AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	const AController* Controller = GetControllerFromActorInfo();
	check(Controller);

	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(FoundHits);

	FGameplayAbilityTargetDataHandle TargetData;
	TargetData.UniqueId = FMath::Rand();

	// Fill out the target data from the hit results
	if (FoundHits.Num() > 0)
	{
		const int32 CartridgeID = FMath::Rand();
		for (const FHitResult& FoundHit : FoundHits)
		{
			FGameplayAbilityTargetData_STH* NewTargetData = new FGameplayAbilityTargetData_STH();
			NewTargetData->HitResult = FoundHit;
			NewTargetData->CartridgeID = CartridgeID;
			TargetData.Add(NewTargetData);
		}
	}
	NotifyTargetDataReady(TargetData, FGameplayTag());
}

FProjectileSpawnTransform UFireAbility::UseAbility(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag)
{
	// retrieve data
	const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(0);
	if (!TargetData)
	{
		// client sent us bad data
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return FProjectileSpawnTransform{};
	}

	// decode data
	const FHitResult* HitResult = TargetData->GetHitResult();

	// Server: Validate data
	const bool bIsServer = CurrentActorInfo->IsNetAuthority();
	if (bIsServer)
	{
		// if (!HitResult->bBlockingHit) // if it wasnt a blocking hit
		// {
		// 	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		// 	return;
		// }
	}

	return GetProjectileSpawnTransform(HitResult, TargetDataHandle);
	//////////////////////////////////////////////////////////////////////
	// Client & Server both -- data is valid, activate the ability with it
	//////////////////////////////////////////////////////////////////////

	// this is an instant ability, end it immediately (only replicate if bIsServer)
	// EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bIsServer, false);
}

FProjectileSpawnTransform UFireAbility::GetProjectileSpawnTransform(const FHitResult* HitResult, const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	const USkeletalMeshComponent* SkeletalMeshComponent = GetWeaponInstance()->GetWeaponMesh();
	if (!SkeletalMeshComponent) return FProjectileSpawnTransform{};

	const USkeletalMeshSocket* MuzzleFlashSocket = SkeletalMeshComponent->GetSocketByName(FName("MuzzleFlash"));
	const UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(SkeletalMeshComponent);
		const FVector ToTarget = HitResult->Location - SocketTransform.GetLocation();
		const FRotator TargetRotation = ToTarget.Rotation();
		
		APawn* InstigatorPawn = Cast<APawn>(CurrentActorInfo->OwnerActor);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = CurrentActorInfo->OwnerActor->GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		return {SocketTransform.GetLocation(), TargetRotation, InstigatorPawn};
	}
	return FProjectileSpawnTransform{};
}

void UFireAbility::PerformLocalTargeting(TArray<FHitResult>& OutHits)
{
	const APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (AvatarPawn && AvatarPawn->IsLocallyControlled())
	{
		FRangedWeaponFiringInput InputData;
		
		const FTransform TargetTransform = GetTargetingTransform(AvatarPawn);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();
		InputData.EndAim = InputData.StartTrace + InputData.AimDir * TRACE_LENGTH;
		
		TraceBullets(InputData, OutHits);
	}
}

FTransform UFireAbility::GetTargetingTransform(const APawn* SourcePawn)
{
	check(SourcePawn);
	const FQuat AimQuat = SourcePawn->GetActorQuat();
	FVector SourceLoc;

	if (AController* Controller = SourcePawn->Controller)
	{
		FVector CamLoc;
		FRotator CamRot;
		constexpr double FocalDistance = 1024.0f;
		const APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC != nullptr)
		{
			PC->GetPlayerViewPoint(CamLoc, CamRot);
		}
		else
		{
			SourceLoc = GetWeaponTargetingSourceLocation();
			CamLoc = SourceLoc;
			CamRot = Controller->GetControlRotation();
		}

		// Determine initial focal point to 
		const FVector AimDir = CamRot.Vector().GetSafeNormal();
		FVector FocalLoc = CamLoc + AimDir * FocalDistance;

		// Move the start and focal point up in front of pawn
		if (PC)
		{
			const FVector WeaponLoc = GetWeaponTargetingSourceLocation();
			CamLoc = FocalLoc + (WeaponLoc - FocalLoc | AimDir) * AimDir;
			FocalLoc = CamLoc + AimDir * FocalDistance;
		}
		return FTransform(CamRot, CamLoc);
	}
	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}

void UFireAbility::TraceBullets(const FRangedWeaponFiringInput& InputData, TArray<FHitResult>& OutHits)
{
	TArray<FHitResult> AllImpacts;
	FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, InputData.EndAim, AllImpacts);
	if (Impact.GetActor() && AllImpacts.Num() > 0)
	{
		OutHits.Append(AllImpacts);
	}

	if (OutHits.Num() == 0)
	{
		if (!Impact.bBlockingHit)
		{
			// Locate the fake 'impact' at the end of the trace
			Impact.Location = InputData.EndAim;
			Impact.ImpactPoint = InputData.EndAim;
		}
		OutHits.Add(Impact);
	}
}

FHitResult UFireAbility::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace,
                                             TArray<FHitResult>& OutHits) const
{
	FHitResult Impact;

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = WeaponTrace(StartTrace, EndTrace, /*out*/ OutHits);
	}
	return Impact;
}

int32 UFireAbility::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			// If we hit a pawn, we're good
			return Idx;
		}
		const AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor && HitActor->GetAttachParentActor() != nullptr && Cast<APawn>(HitActor->GetAttachParentActor()) !=
			nullptr)
		{
			// If we hit something attached to a pawn, we're good
			return Idx;
		}
	}
	return INDEX_NONE;
}

FHitResult UFireAbility::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace,
                                     TArray<FHitResult>& OutHitResults) const
{
	FHitResult HitResult;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);

	if (const UWorld* World = GetWorld())
	{
		World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams);
	}

	// DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, 3.f, 0, 3.f);

	if (HitResult.bBlockingHit)
	{
		OutHitResults.Add(HitResult);
	}

	return HitResult;
}

void UFireAbility::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (const AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

FVector UFireAbility::GetWeaponTargetingSourceLocation()
{
	// Use Pawn's location as a base
	const APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	check(AvatarPawn);

	const FVector SourceLoc = AvatarPawn->GetActorLocation();
	const FVector TargetingSourceLocation = SourceLoc;

	return TargetingSourceLocation;
}

bool UFireAbility::HasEnoughAmmo()
{
	if (const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(); AmmoAttribute.IsValid())
	{
		return AbilitySystemComponent->GetNumericAttribute(AmmoAttribute) > 0.f;
	}
	return false;
}