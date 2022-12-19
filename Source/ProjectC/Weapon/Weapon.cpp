#include "Weapon.h"

#include "AbilitySystemComponent.h"
#include "Casing.h"
#include "GameplayAbilitySpec.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "ProjectC/Character/MannequinCharacter.h"
#include "ProjectC/Components/CardAbilitySystemComponent.h"
#include "ProjectC/Components/CombatComponent.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
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
	DOREPLIFETIME_CONDITION(AWeapon, WeaponOwnerCharacter, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly)
}

void AWeapon::SetOwningCharacter(AMannequinCharacter* Character)
{
	WeaponOwnerCharacter = Character;
	if (WeaponOwnerCharacter)
	{
		AbilitySystemComponent = Cast<UCardAbilitySystemComponent>(WeaponOwnerCharacter->GetAbilitySystemComponent());
		SetOwner(Character);
	}
	else
	{
		AbilitySystemComponent = nullptr;
		SetOwner(nullptr);
	}
}

void AWeapon::AddAbilities()
{
	if (!IsValid(WeaponOwnerCharacter) || !WeaponOwnerCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UCardAbilitySystemComponent* ASC = Cast<UCardAbilitySystemComponent>(
		WeaponOwnerCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		return;
	}
	// Grant abilities, but only on the server	
	if (HasAuthority())
	{
		DefaultAbilities->GiveToAbilitySystem(ASC, this);
	}
}

void AWeapon::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributeEffect)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			DefaultAttributeEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
				*SpecHandle.Data.Get());
		}
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		if (const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject")))
		{
			const FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			if (UWorld* World = GetWorld())
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
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		Sequence++;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	Sequence--;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr
		                       ? Cast<AMannequinCharacter>(GetOwner())
		                       : WeaponOwnerCharacter;
	if (WeaponOwnerCharacter && WeaponOwnerCharacter->GetCombat() && IsFull())
	{
		WeaponOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr
		                       ? Cast<AMannequinCharacter>(GetOwner())
		                       : WeaponOwnerCharacter;
	if (WeaponOwnerCharacter)
	{
		WeaponOwnerController = WeaponOwnerController == nullptr
			                        ? Cast<AMannequinPlayerController>(WeaponOwnerCharacter->Controller)
			                        : WeaponOwnerController;
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

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	// DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Cyan,
	//               true);

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PollInit();
}

void AWeapon::PollInit()
{
	if (WeaponOwnerController == nullptr && HasAuthority() && WeaponOwnerCharacter->Controller)
	{
		WeaponOwnerController = WeaponOwnerController == nullptr
			                        ? Cast<AMannequinPlayerController>(WeaponOwnerCharacter->Controller)
			                        : WeaponOwnerController;
		if (WeaponOwnerController && !WeaponOwnerController->HighPingDelegate.IsBound())
		{
			WeaponOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

UAbilitySystemComponent* AWeapon::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
