#include "MannequinCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/ProjectC.h"
#include "ProjectC/Combat/CombatComponent.h"
#include "ProjectC/GameMode/MatchGameMode.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"
#include "ProjectC/Weapon/Weapon.h"

AMannequinCharacter::AMannequinCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AMannequinCharacter::BeginPlay()
{
	Super::BeginPlay();
	Combat->SpawnWeaponOnCharacter();
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMannequinCharacter::ReceiveDamage);
	}
}

void AMannequinCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
	PollInit();
}

void AMannequinCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMannequinCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMannequinCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMannequinCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMannequinCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMannequinCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMannequinCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMannequinCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMannequinCharacter::LookUp);
}

void AMannequinCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinCharacter, Health)
	DOREPLIFETIME(AMannequinCharacter, bDisableGameplay)
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

void AMannequinCharacter::PlayReloadMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		case EWeaponType::EWT_MAX:
			SectionName = FName("GrenadeLauncher");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMannequinCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AMannequinCharacter::Elim()
{
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AMannequinCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void AMannequinCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance1 && DissolveMaterialInstance2)
	{
		DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance1, this);
		DynamicDissolveMaterialInstance2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance2, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance1);
		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance2);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Glow"), 200.f);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetEquippedWeapon()->Destroy();
}

void AMannequinCharacter::ElimTimerFinished()
{
	AMatchGameMode* MatchGameMode = GetWorld()->GetAuthGameMode<AMatchGameMode>();
	if (MatchGameMode)
	{
		MatchGameMode->RequestRespawn(this, Controller);
	}
}

void AMannequinCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactMontage && !AnimInstance->IsAnyMontagePlaying())
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMannequinCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                        AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		AMatchGameMode* MatchGameMode = GetWorld()->GetAuthGameMode<AMatchGameMode>();
		if (MatchGameMode)
		{
			MannequinPlayerController = MannequinPlayerController == nullptr
				                            ? Cast<AMannequinPlayerController>(Controller)
				                            : MannequinPlayerController;
			AMannequinPlayerController* AttackerController = Cast<AMannequinPlayerController>(InstigatorController);
			MatchGameMode->PlayerEliminated(this, MannequinPlayerController, AttackerController);
		}
	}
}

void AMannequinCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMannequinCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
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
	if (bDisableGameplay) return;
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
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AMannequinCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AMannequinCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void AMannequinCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	if (!bElimmed)
	{
		PlayHitReactMontage();
	}
}

void AMannequinCharacter::UpdateHUDHealth()
{
	MannequinPlayerController = MannequinPlayerController == nullptr
		                            ? Cast<AMannequinPlayerController>(Controller)
		                            : MannequinPlayerController;
	if (MannequinPlayerController)
	{
		MannequinPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AMannequinCharacter::PollInit()
{
	if (MannequinPlayerState == nullptr)
	{
		MannequinPlayerState = GetPlayerState<AMannequinPlayerState>();
		if (MannequinPlayerState)
		{
			MannequinPlayerState->AddToScore(0.f);
			MannequinPlayerState->AddToDefeats(0);
		}
	}
}

void AMannequinCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance1 && DynamicDissolveMaterialInstance2)
	{
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AMannequinCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AMannequinCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
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

ECombatState AMannequinCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}