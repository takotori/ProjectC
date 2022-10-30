#include "MannequinCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectC/ProjectC.h"
#include "ProjectC/Combat/CombatComponent.h"
#include "ProjectC/Weapon/Weapon.h"

AMannequinCharacter::AMannequinCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), "head");
	FollowCamera->SetRelativeLocation(FVector(10, 20, 0));
	FollowCamera->SetRelativeRotation(FRotator(0, 90, -90));
	FollowCamera->bUsePawnControlRotation = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);


	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void AMannequinCharacter::BeginPlay()
{
	Super::BeginPlay();
	Combat->SpawnWeaponOnCharacter();
}


void AMannequinCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMannequinCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMannequinCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMannequinCharacter::FireButtonReleased);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMannequinCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMannequinCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMannequinCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMannequinCharacter::LookUp);
}

void AMannequinCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AMannequinCharacter::PlayFireMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
		
	}
}

void AMannequinCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
		
	}
}

void AMannequinCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMannequinCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMannequinCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMannequinCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMannequinCharacter::CrouchButtonPressed()
{
	bIsCrouched ? UnCrouch() : Crouch();
}

void AMannequinCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	const float Speed = Velocity.Size();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //standing still
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
	}

	if (Speed > 0.f || bIsInAir) // moving
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FRotator BaseAimRotation = GetBaseAimRotation();
		BaseAimRotation.Normalize();
		AO_Pitch = BaseAimRotation.Pitch;
	}
}

void AMannequinCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AMannequinCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AMannequinCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

AWeapon* AMannequinCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AMannequinCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();

	return Combat->HitTarget;
 	
}

void AMannequinCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
}
