#include "MannequinCharacter.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/CharacterInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ProjectC/ProjectC.h"
#include "ProjectC/Abilities/BaseAbility.h"
#include "ProjectC/Abilities/AttributeSets/CharacterAttributes.h"
#include "ProjectC/Components//BuffComponent.h"
#include "ProjectC/Components/CardAbilitySystemComponent.h"
#include "ProjectC/Components/CombatComponent.h"
#include "ProjectC/Components/LagCompensationComponent.h"
#include "ProjectC/GameMode/MatchGameMode.h"
#include "ProjectC/PlayerController/MannequinPlayerController.h"
#include "ProjectC/PlayerState/MannequinPlayerState.h"
#include "ProjectC/Weapon/Weapon.h"

AMannequinCharacter::AMannequinCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), "head");
	FollowCamera->SetRelativeLocation(FVector(10, 20, 0));
	FollowCamera->SetRelativeRotation(FRotator(0, 90, -90));
	FollowCamera->bUsePawnControlRotation = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("Lag Compensation"));

	AbilitySystemComponent = CreateDefaultSubobject<UCardAbilitySystemComponent>(TEXT("Ability Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UCharacterAttributes>(TEXT("Attributes"));

	CreateBoxComponents();
	for (const auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AMannequinCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMannequinCharacter::ReceiveDamage);
	}
	if (AbilitySystemComponent)
	{
		AttributeSet = AbilitySystemComponent->GetSet<UCharacterAttributes>();
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void AMannequinCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	const FVector2d MovementVector = Value.Get<FVector2d>();
	if (Controller != nullptr)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

		const FVector ForwardDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(ForwardDirection, MovementVector.Y);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMannequinCharacter::Look(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	const FVector2D LookAxisVector = Value.Get<FVector2d>();
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
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

	if (UEnhancedInputComponent* InComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		TArray<uint32> BindHandles;

		for (const FCharacterInputAction& Action : InputConfig->AbilityInputActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid())
			{
				if (&AMannequinCharacter::AbilityInputTagPressed)
				{
					BindHandles.Add(InComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, this,
					                                        &AMannequinCharacter::AbilityInputTagPressed,
					                                        Action.InputTag).GetHandle());
				}

				if (&AMannequinCharacter::AbilityInputTagReleased)
				{
					BindHandles.Add(InComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, this,
					                                        &AMannequinCharacter::AbilityInputTagReleased,
					                                        Action.InputTag).GetHandle());
				}
			}
		}

		InComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMannequinCharacter::Move);
		InComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMannequinCharacter::Look);
	}
}

void AMannequinCharacter::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagPressed(InputTag);
	}
}

void AMannequinCharacter::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagReleased(InputTag);
	}
}

void AMannequinCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinCharacter, Health)
	DOREPLIFETIME(AMannequinCharacter, Shield)
	DOREPLIFETIME(AMannequinCharacter, bDisableGameplay)
}

void AMannequinCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<APlayerController>(Controller);
		}
	}
}

void AMannequinCharacter::PlayFireMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		const FName SectionName = FName("RifleHip");
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
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Submachinegun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("GrenadeLauncher");
			break;
		case EWeaponType::EWT_Sniper:
			SectionName = FName("Rifle");
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

void AMannequinCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void AMannequinCharacter::Elim(bool bPlayerLeftGame)
{
	MulticastElim(bPlayerLeftGame);
}

void AMannequinCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
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
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AMannequinCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void AMannequinCharacter::ElimTimerFinished()
{
	MatchGameMode = MatchGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMatchGameMode>() : MatchGameMode;
	if (MatchGameMode && !bLeftGame)
	{
		MatchGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void AMannequinCharacter::ServerLeaveGame_Implementation()
{
	MatchGameMode = MatchGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMatchGameMode>() : MatchGameMode;
	MannequinPlayerState = MannequinPlayerState == nullptr
		                       ? GetPlayerState<AMannequinPlayerState>()
		                       : MannequinPlayerState;
	if (MatchGameMode && MannequinPlayerState)
	{
		MatchGameMode->PlayerLeftGame(MannequinPlayerState);
	}
}

void AMannequinCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage && !AnimInstance->IsAnyMontagePlaying())
	{
		AnimInstance->Montage_Play(HitReactMontage);
		const FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMannequinCharacter::ThrowGrenade()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void AMannequinCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                        AController* InstigatorController, AActor* DamageCauser)
{
	MatchGameMode = MatchGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMatchGameMode>() : MatchGameMode;
	if (bElimmed || MatchGameMode == nullptr) return;
	Damage = MatchGameMode->CalculateDamage(InstigatorController, Controller, Damage);

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
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

void AMannequinCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	const float Speed = Velocity.Size();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) //standing still
	{
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(
			CurrentAimRotation, StartingAimRotation);
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

void AMannequinCharacter::Fire()
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

void AMannequinCharacter::Reload()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void AMannequinCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth && !bElimmed)
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

void AMannequinCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield && !bElimmed)
	{
		PlayHitReactMontage();
	}
}

void AMannequinCharacter::UpdateHUDShield()
{
	MannequinPlayerController = MannequinPlayerController == nullptr
		                            ? Cast<AMannequinPlayerController>(Controller)
		                            : MannequinPlayerController;
	if (MannequinPlayerController)
	{
		MannequinPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void AMannequinCharacter::UpdateHUDAmmo()
{
	MannequinPlayerController = MannequinPlayerController == nullptr
		                            ? Cast<AMannequinPlayerController>(Controller)
		                            : MannequinPlayerController;
	if (MannequinPlayerController && Combat && Combat->EquippedWeapon)
	{
		MannequinPlayerController->SetHUDAmmo(GetEquippedWeapon()->GetAmmo());
	}
}

void AMannequinCharacter::SpawnDefaultWeapon()
{
	MatchGameMode = MatchGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMatchGameMode>() : MatchGameMode;
	UWorld* World = GetWorld();
	if (MatchGameMode && World && !bElimmed && DefaultWeaponClass && Combat)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->SetOwningCharacter(this);
		StartingWeapon->InitializeAttributes();
		Combat->EquipWeapon(StartingWeapon);
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
	if (MannequinPlayerController == nullptr)
	{
		MannequinPlayerController = MannequinPlayerController == nullptr
			                            ? Cast<AMannequinPlayerController>(Controller)
			                            : MannequinPlayerController;
		if (MannequinPlayerController)
		{
			UpdateHUDHealth();
			UpdateHUDShield();
			UpdateHUDAmmo();
			if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
					UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				{
					Subsystem->AddMappingContext(BaseMappingContext, 0);
				}
			}
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

void AMannequinCharacter::InitializeAttributes()
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

void AMannequinCharacter::GiveAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		SpawnDefaultWeapon();
		DefaultAbilities->GiveToAbilitySystem(AbilitySystemComponent, this);
	}
}

void AMannequinCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//Server GAS init
	MannequinPlayerState = MannequinPlayerState == nullptr
		                       ? GetPlayerState<AMannequinPlayerState>()
		                       : MannequinPlayerState;
	if (MannequinPlayerState)
	{
		AbilitySystemComponent = Cast<UCardAbilitySystemComponent>(MannequinPlayerState->GetAbilitySystemComponent());
		MannequinPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(MannequinPlayerState, this);
	}
	InitializeAttributes();
	GiveAbilities();
}

void AMannequinCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	MannequinPlayerState = MannequinPlayerState == nullptr
		                       ? GetPlayerState<AMannequinPlayerState>()
		                       : MannequinPlayerState;
	if (MannequinPlayerState)
	{
		AbilitySystemComponent = Cast<UCardAbilitySystemComponent>(MannequinPlayerState->GetAbilitySystemComponent());
		MannequinPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(MannequinPlayerState, this);
	}
	InitializeAttributes();
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

bool AMannequinCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

void AMannequinCharacter::CreateBoxComponents()
{
	// Hit boxes for server-side rewind
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);
}

UAbilitySystemComponent* AMannequinCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
