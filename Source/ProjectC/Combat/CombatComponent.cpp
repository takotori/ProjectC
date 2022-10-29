

#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonpressed = bPressed;
	if (EquippedWeapon == nullptr) return;
	
	if (Character && bFireButtonpressed)
	{
		Character->PlayFireMontage();
		EquippedWeapon->Fire();
	}
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::SpawnWeaponOnCharacter()
{
	if (Character == nullptr && !GetWorld()) return;
	
	EquippedWeapon = Cast<AWeapon>(GetWorld()->SpawnActor(WeaponToSpawn));

	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		EquippedWeapon->SetOwner(Character);
	}	
}