// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Casing.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	// set weapon collision if player can drop weapon
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// area sphere is to pickup weapon if player gets close enough
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<AMannequinCharacter>(GetOwner()) : WeaponOwnerCharacter;
	if (WeaponOwnerCharacter)
	{
		WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<AMannequinPlayerController>(WeaponOwnerCharacter->Controller) : WeaponOwnerController;
		if (WeaponOwnerController)
		{
			WeaponOwnerController->SetHUDAmmo(Ammo);
		}
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
