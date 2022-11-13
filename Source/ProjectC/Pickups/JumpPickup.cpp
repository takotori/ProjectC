
#include "JumpPickup.h"

#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Components/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(OtherActor);
	if (MannequinCharacter)
	{
		UBuffComponent* Buff = MannequinCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffJump(JumpZVelocityBuff, JumpBuffTime);
		}
	}
	Destroy();
}
