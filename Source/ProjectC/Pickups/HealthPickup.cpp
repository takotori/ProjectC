#include "HealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Components/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                    const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AMannequinCharacter* MannequinCharacter = Cast<AMannequinCharacter>(OtherActor);
	if (MannequinCharacter)
	{
		UBuffComponent* Buff = MannequinCharacter->GetBuff();
		if (Buff)
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}
	Destroy();
}

void AHealthPickup::Destroyed()
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
	Super::Destroyed();
}
