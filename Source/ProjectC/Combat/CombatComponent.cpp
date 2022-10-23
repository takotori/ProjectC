

#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCombatComponent::EquipWeapon()
{
	if (Character == nullptr) return;

	AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>();
	
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(Weapon, Character->GetMesh());
	}
	Weapon->SetOwner(Character);
	
}