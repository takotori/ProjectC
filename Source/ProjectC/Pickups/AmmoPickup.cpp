
#include "AmmoPickup.h"

#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Combat/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(OtherActor);
	if (MannequinCharacter)
	{
		UCombatComponent* Combat = MannequinCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
