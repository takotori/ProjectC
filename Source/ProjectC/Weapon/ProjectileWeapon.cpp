#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority()) // server
			{
				if (InstigatorPawn->IsLocallyControlled()) // server, host - use replicated projectile
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
				}
				else // server, not locally controlled - spawn non-replicated projectile, ssr
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ServerSideRewindProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			else // client, using ssr
			{
				if (InstigatorPawn->IsLocallyControlled())
				// client, locally controlled - spawn non-replicated projectile, use ssr
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ServerSideRewindProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile
						->InitialSpeed;
					SpawnedProjectile->Damage = Damage;
				}
				else // client, not locally controlled - spawn non-replicated projectile, no ssr
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						ServerSideRewindProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else // weapon not using ssr
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
				);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
		}
	}
}
